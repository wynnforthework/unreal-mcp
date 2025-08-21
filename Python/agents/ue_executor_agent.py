"""
UE Editor Execution Agent

Takes UMG JSON and executes Python scripts in UE5.6 to generate actual Widget Blueprints.
This agent interfaces with the existing UMG MCP tools and Unreal Python API.
"""

import asyncio
import json
import time
import socket
from typing import Dict, Any, List, Optional, Tuple
from dataclasses import dataclass

from base_agent import BaseAgent, AgentResult, AgentStatus
from schemas import (
    UE_EXECUTION_INPUT_SCHEMA,
    UE_EXECUTION_OUTPUT_SCHEMA
)


@dataclass
class ComponentCreationResult:
    """Result of creating a single component"""
    name: str
    type: str
    success: bool
    error_message: Optional[str] = None


@dataclass
class EventBindingResult:
    """Result of creating an event binding"""
    widget_name: str
    event_type: str
    function_name: str
    success: bool
    error_message: Optional[str] = None


class UEExecutorAgent(BaseAgent):
    """
    Agent that executes UMG widget creation in Unreal Engine.
    
    This agent:
    - Creates Widget Blueprints using existing UMG MCP tools
    - Adds components to the widget hierarchy
    - Sets up event bindings and properties
    - Compiles and validates the resulting blueprint
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        super().__init__("ue_executor", config)
        
        # TCP connection settings for Unreal Engine
        self.tcp_host = config.get("tcp_host", "127.0.0.1") if config else "127.0.0.1"
        self.tcp_port = config.get("tcp_port", 55557) if config else 55557
        self.connection_timeout = config.get("connection_timeout", 10.0) if config else 10.0
        
        # Execution settings
        self.max_retries = config.get("max_retries", 3) if config else 3
        self.retry_delay = config.get("retry_delay", 1.0) if config else 1.0
    
    async def execute(self, input_data: Dict[str, Any]) -> AgentResult:
        """Execute the UE widget creation process"""
        start_time = time.time()
        
        try:
            # Validate input
            if not self.validate_input(input_data):
                return self.create_error_result(
                    "INVALID_INPUT",
                    "Input data does not match required schema"
                )
            
            umg_definition = input_data["umg_widget_definition"]
            
            self.logger.info(f"Creating widget blueprint: {umg_definition['widget_name']}")
            
            # Execute widget creation workflow
            execution_result = await self._execute_widget_creation(umg_definition)
            
            # Calculate performance metrics
            performance_metrics = self._calculate_performance_metrics(
                execution_result, time.time() - start_time
            )
            
            # Create result
            result_data = {
                "execution_result": execution_result,
                "performance_metrics": performance_metrics
            }
            
            execution_time = time.time() - start_time
            return self.create_success_result(result_data, execution_time=execution_time)
            
        except Exception as e:
            execution_time = time.time() - start_time
            self.logger.error(f"Error executing widget creation: {e}")
            return self.create_error_result(
                "EXECUTION_ERROR",
                f"Failed to execute widget creation: {str(e)}",
                execution_time=execution_time
            )
    
    def validate_input(self, input_data: Dict[str, Any]) -> bool:
        """Validate input data against schema"""
        if "umg_widget_definition" not in input_data:
            return False
        
        umg_def = input_data["umg_widget_definition"]
        required_fields = ["widget_name", "root_component"]
        
        for field in required_fields:
            if field not in umg_def:
                return False
        
        return True
    
    async def _execute_widget_creation(self, umg_definition: Dict[str, Any]) -> Dict[str, Any]:
        """Execute the complete widget creation workflow"""
        
        widget_name = umg_definition["widget_name"]
        parent_class = umg_definition.get("parent_class", "UserWidget")
        path = umg_definition.get("path", "/Game/Widgets")
        root_component = umg_definition["root_component"]
        cpp_bindings = umg_definition.get("cpp_bindings", [])
        event_bindings = umg_definition.get("event_bindings", [])
        
        execution_result = {
            "widget_blueprint_path": f"{path}/{widget_name}",
            "created_components": [],
            "event_bindings_created": [],
            "compilation_status": "unknown",
            "compilation_errors": []
        }
        
        try:
            # Step 1: Create the widget blueprint
            self.logger.info(f"Creating widget blueprint: {widget_name}")
            create_result = await self._send_ue_command("create_umg_widget_blueprint", {
                "widget_name": widget_name,
                "parent_class": parent_class,
                "path": path
            })
            
            if not create_result.get("success", False):
                raise Exception(f"Failed to create widget blueprint: {create_result.get('error', 'Unknown error')}")
            
            execution_result["widget_blueprint_path"] = create_result.get("widget_path", f"{path}/{widget_name}")
            
            # Step 2: Create components recursively
            self.logger.info("Creating widget components")
            created_components = await self._create_components_recursive(
                widget_name, root_component, None
            )
            execution_result["created_components"] = [comp.__dict__ for comp in created_components]
            
            # Step 3: Set up event bindings
            self.logger.info("Setting up event bindings")
            event_results = await self._create_event_bindings(widget_name, event_bindings)
            execution_result["event_bindings_created"] = [result.__dict__ for result in event_results]
            
            # Step 4: Compile the blueprint
            self.logger.info("Compiling widget blueprint")
            compile_result = await self._compile_widget_blueprint(widget_name)
            execution_result["compilation_status"] = compile_result.get("status", "unknown")
            execution_result["compilation_errors"] = compile_result.get("errors", [])
            
            return execution_result
            
        except Exception as e:
            self.logger.error(f"Widget creation failed: {e}")
            execution_result["compilation_status"] = "failed"
            execution_result["compilation_errors"] = [str(e)]
            return execution_result
    
    async def _create_components_recursive(self, widget_name: str, component: Dict[str, Any], 
                                         parent_name: Optional[str]) -> List[ComponentCreationResult]:
        """Create components recursively in the widget hierarchy"""
        
        results = []
        
        # Create current component (skip root canvas if it's the default)
        if parent_name is not None or component["type"] != "CanvasPanel":
            result = await self._create_single_component(widget_name, component, parent_name)
            results.append(result)
            current_component_name = component["name"]
        else:
            # Root canvas panel - use as parent for children
            current_component_name = None
        
        # Create children
        for child in component.get("children", []):
            child_results = await self._create_components_recursive(
                widget_name, child, current_component_name or component["name"]
            )
            results.extend(child_results)
        
        return results
    
    async def _create_single_component(self, widget_name: str, component: Dict[str, Any], 
                                     parent_name: Optional[str]) -> ComponentCreationResult:
        """Create a single widget component"""
        
        try:
            component_name = component["name"]
            component_type = component["type"]
            position = component.get("position", {"x": 0.0, "y": 0.0})
            size = component.get("size", {"width": 100.0, "height": 50.0})
            properties = component.get("properties", {})
            
            # Prepare command parameters
            command_params = {
                "widget_name": widget_name,
                "component_name": component_name,
                "component_type": component_type,
                "position": [position["x"], position["y"]],
                "size": [size["width"], size["height"]]
            }
            
            # Add properties if any
            if properties:
                command_params["kwargs"] = properties
            
            # Choose appropriate command based on whether we have a parent
            if parent_name:
                # Use parent-child creation command
                command_params["parent_component_name"] = parent_name
                command_params["child_component_name"] = component_name
                command_params["parent_component_type"] = "Border"  # Default parent type
                command_params["child_component_type"] = component_type
                command_params["child_attributes"] = properties
                
                result = await self._send_ue_command("create_parent_and_child_widget_components", command_params)
            else:
                # Use direct component creation
                result = await self._send_ue_command("add_widget_component", command_params)
            
            if result.get("success", False):
                self.logger.debug(f"Created component: {component_name} ({component_type})")
                return ComponentCreationResult(
                    name=component_name,
                    type=component_type,
                    success=True
                )
            else:
                error_msg = result.get("error", "Unknown error")
                self.logger.warning(f"Failed to create component {component_name}: {error_msg}")
                return ComponentCreationResult(
                    name=component_name,
                    type=component_type,
                    success=False,
                    error_message=error_msg
                )
                
        except Exception as e:
            self.logger.error(f"Exception creating component {component.get('name', 'unknown')}: {e}")
            return ComponentCreationResult(
                name=component.get("name", "unknown"),
                type=component.get("type", "unknown"),
                success=False,
                error_message=str(e)
            )
    
    async def _create_event_bindings(self, widget_name: str, 
                                   event_bindings: List[Dict[str, Any]]) -> List[EventBindingResult]:
        """Create event bindings for widget components"""
        
        results = []
        
        for binding in event_bindings:
            try:
                widget_component_name = binding["widget_name"]
                event_type = binding["event_type"]
                function_name = binding["function_name"]
                
                # Create event binding
                command_params = {
                    "widget_name": widget_name,
                    "component_name": widget_component_name,
                    "event_type": event_type,
                    "function_name": function_name
                }
                
                result = await self._send_ue_command("bind_widget_event", command_params)
                
                if result.get("success", False):
                    self.logger.debug(f"Created event binding: {widget_component_name}.{event_type} -> {function_name}")
                    results.append(EventBindingResult(
                        widget_name=widget_component_name,
                        event_type=event_type,
                        function_name=function_name,
                        success=True
                    ))
                else:
                    error_msg = result.get("error", "Unknown error")
                    self.logger.warning(f"Failed to create event binding: {error_msg}")
                    results.append(EventBindingResult(
                        widget_name=widget_component_name,
                        event_type=event_type,
                        function_name=function_name,
                        success=False,
                        error_message=error_msg
                    ))
                    
            except Exception as e:
                self.logger.error(f"Exception creating event binding: {e}")
                results.append(EventBindingResult(
                    widget_name=binding.get("widget_name", "unknown"),
                    event_type=binding.get("event_type", "unknown"),
                    function_name=binding.get("function_name", "unknown"),
                    success=False,
                    error_message=str(e)
                ))
        
        return results
    
    async def _compile_widget_blueprint(self, widget_name: str) -> Dict[str, Any]:
        """Compile the widget blueprint and return compilation status"""
        
        try:
            # Use blueprint compilation command if available
            result = await self._send_ue_command("compile_blueprint", {
                "blueprint_name": widget_name
            })
            
            if result.get("success", False):
                return {
                    "status": "success",
                    "errors": []
                }
            else:
                return {
                    "status": "failed",
                    "errors": [result.get("error", "Compilation failed")]
                }
                
        except Exception as e:
            self.logger.warning(f"Could not compile blueprint {widget_name}: {e}")
            return {
                "status": "unknown",
                "errors": [str(e)]
            }
    
    async def _send_ue_command(self, command_type: str, params: Dict[str, Any]) -> Dict[str, Any]:
        """Send a command to Unreal Engine via TCP"""
        
        for attempt in range(self.max_retries):
            try:
                # Create command payload
                command_data = {
                    "type": command_type,
                    "params": params
                }
                
                json_data = json.dumps(command_data)
                
                # Connect to UE TCP server
                reader, writer = await asyncio.wait_for(
                    asyncio.open_connection(self.tcp_host, self.tcp_port),
                    timeout=self.connection_timeout
                )
                
                # Send command
                writer.write(json_data.encode('utf-8'))
                writer.write(b'\n')
                await writer.drain()
                
                # Read response
                response_data = await asyncio.wait_for(
                    reader.read(8192),
                    timeout=self.connection_timeout
                )
                response_str = response_data.decode('utf-8').strip()
                
                # Close connection
                writer.close()
                await writer.wait_closed()
                
                # Parse response
                if response_str:
                    return json.loads(response_str)
                else:
                    return {"success": False, "error": "Empty response from UE"}
                    
            except asyncio.TimeoutError:
                self.logger.warning(f"Timeout on attempt {attempt + 1} for command {command_type}")
                if attempt < self.max_retries - 1:
                    await asyncio.sleep(self.retry_delay)
                    continue
                return {"success": False, "error": "Connection timeout"}
                
            except Exception as e:
                self.logger.warning(f"Error on attempt {attempt + 1} for command {command_type}: {e}")
                if attempt < self.max_retries - 1:
                    await asyncio.sleep(self.retry_delay)
                    continue
                return {"success": False, "error": f"Communication error: {str(e)}"}
        
        return {"success": False, "error": "Max retries exceeded"}
    
    def _calculate_performance_metrics(self, execution_result: Dict[str, Any], 
                                     total_time: float) -> Dict[str, Any]:
        """Calculate performance metrics for the execution"""
        
        component_count = len(execution_result.get("created_components", []))
        binding_count = len(execution_result.get("event_bindings_created", []))
        
        successful_components = sum(
            1 for comp in execution_result.get("created_components", [])
            if comp.get("success", False)
        )
        
        successful_bindings = sum(
            1 for binding in execution_result.get("event_bindings_created", [])
            if binding.get("success", False)
        )
        
        return {
            "creation_time": total_time,
            "component_count": component_count,
            "binding_count": binding_count,
            "successful_components": successful_components,
            "successful_bindings": successful_bindings,
            "success_rate": (successful_components + successful_bindings) / max(component_count + binding_count, 1)
        }
    
    def _get_input_schema(self) -> Dict[str, Any]:
        """Get input schema for this agent"""
        return UE_EXECUTION_INPUT_SCHEMA
    
    def _get_output_schema(self) -> Dict[str, Any]:
        """Get output schema for this agent"""
        return UE_EXECUTION_OUTPUT_SCHEMA
