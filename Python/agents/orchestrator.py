"""
Agent Orchestrator

Manages the workflow that chains the 4 agents together:
User Input → UI Parser → Design Translator → UE Executor → Binding Validator → Final Report

Includes error handling, rollback mechanisms, and progress tracking.
"""

import asyncio
import json
import time
import uuid
from typing import Dict, Any, List, Optional, Callable, Awaitable
from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum

from .base_agent import BaseAgent, AgentResult, AgentStatus
from .communication import MessageBus, AgentEndpoint, WorkflowState, MessageType
from .ui_parser_agent import UIParserAgent
from .design_translator_agent import DesignTranslatorAgent
from .ue_executor_agent import UEExecutorAgent
from .binding_validator_agent import BindingValidatorAgent


class WorkflowStep(Enum):
    """Steps in the UMG generation workflow"""
    PARSE_REQUIREMENTS = "parse_requirements"
    TRANSLATE_DESIGN = "translate_design"
    EXECUTE_CREATION = "execute_creation"
    VALIDATE_BINDINGS = "validate_bindings"
    GENERATE_REPORT = "generate_report"


@dataclass
class WorkflowConfig:
    """Configuration for the workflow orchestrator"""
    enable_rollback: bool = True
    max_retries_per_step: int = 2
    timeout_per_step: float = 300.0  # 5 minutes
    parallel_validation: bool = False
    auto_fix_errors: bool = False
    cpp_header_path: Optional[str] = None
    ue_tcp_host: str = "127.0.0.1"
    ue_tcp_port: int = 55557


@dataclass
class WorkflowResult:
    """Final result of the complete workflow"""
    workflow_id: str
    status: str  # success, failed, partial
    ui_specification: Optional[Dict[str, Any]] = None
    umg_definition: Optional[Dict[str, Any]] = None
    execution_result: Optional[Dict[str, Any]] = None
    validation_report: Optional[Dict[str, Any]] = None
    final_report: Optional[Dict[str, Any]] = None
    execution_time: float = 0.0
    step_results: List[Dict[str, Any]] = field(default_factory=list)
    errors: List[str] = field(default_factory=list)
    warnings: List[str] = field(default_factory=list)


class AgentOrchestrator:
    """
    Orchestrates the complete UMG generation workflow.
    
    Manages the execution flow between agents, handles errors,
    provides rollback capabilities, and tracks progress.
    """
    
    def __init__(self, config: Optional[WorkflowConfig] = None):
        self.config = config or WorkflowConfig()
        self.message_bus = MessageBus()
        self.workflow_state: Optional[WorkflowState] = None
        
        # Initialize agents
        agent_config = {
            "tcp_host": self.config.ue_tcp_host,
            "tcp_port": self.config.ue_tcp_port
        }
        
        self.ui_parser = UIParserAgent()
        self.design_translator = DesignTranslatorAgent()
        self.ue_executor = UEExecutorAgent(agent_config)
        self.binding_validator = BindingValidatorAgent(agent_config)
        
        # Setup agent endpoints
        self.endpoints = {}
        self._setup_agent_endpoints()
        
        # Workflow step definitions
        self.workflow_steps = [
            (WorkflowStep.PARSE_REQUIREMENTS, self._execute_ui_parsing),
            (WorkflowStep.TRANSLATE_DESIGN, self._execute_design_translation),
            (WorkflowStep.EXECUTE_CREATION, self._execute_ue_creation),
            (WorkflowStep.VALIDATE_BINDINGS, self._execute_binding_validation),
            (WorkflowStep.GENERATE_REPORT, self._generate_final_report)
        ]
        
        self.logger = self.ui_parser.logger  # Use shared logger
    
    def _setup_agent_endpoints(self):
        """Setup communication endpoints for all agents"""
        agents = [
            ("ui_parser", self.ui_parser),
            ("design_translator", self.design_translator),
            ("ue_executor", self.ue_executor),
            ("binding_validator", self.binding_validator)
        ]
        
        for agent_id, agent in agents:
            endpoint = AgentEndpoint(agent_id, self.message_bus)
            self.endpoints[agent_id] = endpoint
    
    async def execute_workflow(self, natural_language_request: str,
                             context: Optional[Dict[str, Any]] = None) -> WorkflowResult:
        """Execute the complete UMG generation workflow"""
        
        workflow_id = str(uuid.uuid4())
        start_time = time.time()
        
        self.logger.info(f"Starting workflow {workflow_id}: {natural_language_request[:100]}...")
        
        # Initialize workflow state
        self.workflow_state = WorkflowState(workflow_id)
        
        # Start message bus
        await self.message_bus.start()
        
        try:
            # Initialize workflow data
            workflow_data = {
                "natural_language_request": natural_language_request,
                "context": context or {},
                "workflow_id": workflow_id
            }
            
            # Execute workflow steps
            result = await self._execute_workflow_steps(workflow_data)
            
            # Calculate total execution time
            result.execution_time = time.time() - start_time
            
            self.logger.info(f"Workflow {workflow_id} completed with status: {result.status}")
            return result
            
        except Exception as e:
            self.logger.error(f"Workflow {workflow_id} failed: {e}")
            
            # Create error result
            result = WorkflowResult(
                workflow_id=workflow_id,
                status="failed",
                execution_time=time.time() - start_time,
                errors=[str(e)]
            )
            
            return result
            
        finally:
            # Stop message bus
            await self.message_bus.stop()
    
    async def _execute_workflow_steps(self, workflow_data: Dict[str, Any]) -> WorkflowResult:
        """Execute all workflow steps in sequence"""
        
        result = WorkflowResult(
            workflow_id=workflow_data["workflow_id"],
            status="running"
        )
        
        current_data = workflow_data.copy()
        
        for i, (step, executor) in enumerate(self.workflow_steps):
            self.logger.info(f"Executing step {i+1}/{len(self.workflow_steps)}: {step.value}")
            
            # Add step to workflow state
            self.workflow_state.add_step(step.value, step.value, current_data)
            self.workflow_state.start_step(i)
            
            try:
                # Execute step with timeout
                step_result = await asyncio.wait_for(
                    executor(current_data),
                    timeout=self.config.timeout_per_step
                )
                
                # Check if step succeeded
                if step_result.is_success:
                    # Update workflow state
                    self.workflow_state.complete_step(i, step_result.data)
                    
                    # Update current data for next step
                    current_data.update(step_result.data)
                    
                    # Store step result
                    result.step_results.append({
                        "step": step.value,
                        "status": "success",
                        "data": step_result.data,
                        "execution_time": step_result.execution_time
                    })
                    
                else:
                    # Step failed
                    error_msg = step_result.error.message if step_result.error else "Unknown error"
                    self.workflow_state.fail_step(i, error_msg)
                    
                    # Handle step failure
                    if await self._handle_step_failure(step, step_result, current_data):
                        # Retry succeeded, continue
                        continue
                    else:
                        # Retry failed, abort workflow
                        result.status = "failed"
                        result.errors.append(f"Step {step.value} failed: {error_msg}")
                        break
                        
            except asyncio.TimeoutError:
                error_msg = f"Step {step.value} timed out after {self.config.timeout_per_step} seconds"
                self.workflow_state.fail_step(i, error_msg)
                result.status = "failed"
                result.errors.append(error_msg)
                break
                
            except Exception as e:
                error_msg = f"Step {step.value} raised exception: {str(e)}"
                self.workflow_state.fail_step(i, error_msg)
                result.status = "failed"
                result.errors.append(error_msg)
                break
        
        # Set final status
        if result.status == "running":
            result.status = "success"
        
        # Extract final results
        self._extract_final_results(result, current_data)
        
        return result
    
    async def _execute_ui_parsing(self, data: Dict[str, Any]) -> AgentResult:
        """Execute UI requirements parsing step"""
        
        input_data = {
            "natural_language_request": data["natural_language_request"],
            "context": data.get("context", {})
        }
        
        return await self.ui_parser.execute(input_data)
    
    async def _execute_design_translation(self, data: Dict[str, Any]) -> AgentResult:
        """Execute design translation step"""
        
        if "ui_specification" not in data:
            return self.ui_parser.create_error_result(
                "MISSING_DATA",
                "UI specification not found from previous step"
            )
        
        input_data = {
            "ui_specification": data["ui_specification"]
        }
        
        return await self.design_translator.execute(input_data)
    
    async def _execute_ue_creation(self, data: Dict[str, Any]) -> AgentResult:
        """Execute UE widget creation step"""
        
        if "umg_widget_definition" not in data:
            return self.ue_executor.create_error_result(
                "MISSING_DATA",
                "UMG widget definition not found from previous step"
            )
        
        input_data = {
            "umg_widget_definition": data["umg_widget_definition"]
        }
        
        return await self.ue_executor.execute(input_data)
    
    async def _execute_binding_validation(self, data: Dict[str, Any]) -> AgentResult:
        """Execute binding validation step"""
        
        execution_result = data.get("execution_result")
        if not execution_result:
            return self.binding_validator.create_error_result(
                "MISSING_DATA",
                "Execution result not found from previous step"
            )
        
        widget_blueprint_path = execution_result.get("widget_blueprint_path")
        if not widget_blueprint_path:
            return self.binding_validator.create_error_result(
                "MISSING_DATA",
                "Widget blueprint path not found in execution result"
            )
        
        input_data = {
            "widget_blueprint_path": widget_blueprint_path,
            "cpp_header_path": self.config.cpp_header_path,
            "expected_bindings": data.get("umg_widget_definition", {}).get("cpp_bindings", [])
        }
        
        return await self.binding_validator.execute(input_data)
    
    async def _generate_final_report(self, data: Dict[str, Any]) -> AgentResult:
        """Generate final workflow report"""
        
        try:
            # Compile final report
            report = {
                "workflow_summary": {
                    "workflow_id": data["workflow_id"],
                    "request": data["natural_language_request"],
                    "status": "completed",
                    "timestamp": datetime.now().isoformat()
                },
                "ui_specification": data.get("ui_specification"),
                "umg_definition": data.get("umg_widget_definition"),
                "execution_result": data.get("execution_result"),
                "validation_report": data.get("validation_report"),
                "recommendations": self._generate_recommendations(data)
            }
            
            return self.ui_parser.create_success_result({"final_report": report})
            
        except Exception as e:
            return self.ui_parser.create_error_result(
                "REPORT_GENERATION_ERROR",
                f"Failed to generate final report: {str(e)}"
            )
    
    def _generate_recommendations(self, data: Dict[str, Any]) -> List[str]:
        """Generate recommendations based on workflow results"""
        
        recommendations = []
        
        # Check validation results
        validation_report = data.get("validation_report")
        if validation_report:
            overall_status = validation_report.get("overall_status", "unknown")
            if overall_status == "error":
                recommendations.append("Fix binding errors before using the widget")
            elif overall_status == "warning":
                recommendations.append("Review binding warnings for optimal integration")
            else:
                recommendations.append("Widget is ready for use!")
        
        # Check execution results
        execution_result = data.get("execution_result")
        if execution_result:
            compilation_status = execution_result.get("compilation_status", "unknown")
            if compilation_status == "failed":
                recommendations.append("Fix compilation errors in the generated blueprint")
            elif compilation_status == "success":
                recommendations.append("Blueprint compiled successfully")
        
        # General recommendations
        recommendations.append("Test the widget in the UE editor")
        recommendations.append("Consider adding custom styling and animations")
        
        return recommendations
    
    async def _handle_step_failure(self, step: WorkflowStep, step_result: AgentResult,
                                 current_data: Dict[str, Any]) -> bool:
        """Handle step failure with retry logic"""
        
        if not self.config.enable_rollback:
            return False
        
        # For now, implement simple retry logic
        # In the future, this could include more sophisticated error recovery
        
        self.logger.warning(f"Step {step.value} failed, attempting rollback and retry")
        
        # Rollback to previous checkpoint if available
        if self.workflow_state.rollback_to_checkpoint():
            self.logger.info("Rollback successful")
            return False  # For now, don't retry automatically
        
        return False
    
    def _extract_final_results(self, result: WorkflowResult, data: Dict[str, Any]):
        """Extract final results from workflow data"""
        
        result.ui_specification = data.get("ui_specification")
        result.umg_definition = data.get("umg_widget_definition")
        result.execution_result = data.get("execution_result")
        result.validation_report = data.get("validation_report")
        result.final_report = data.get("final_report")
    
    def get_workflow_progress(self) -> Optional[Dict[str, Any]]:
        """Get current workflow progress"""
        
        if not self.workflow_state:
            return None
        
        return self.workflow_state.get_progress()
    
    async def cancel_workflow(self) -> bool:
        """Cancel the current workflow"""
        
        if self.workflow_state:
            # Mark current step as cancelled
            current_step = self.workflow_state.current_step
            if current_step < len(self.workflow_state.steps):
                self.workflow_state.fail_step(current_step, "Workflow cancelled by user")
            
            return True
        
        return False
