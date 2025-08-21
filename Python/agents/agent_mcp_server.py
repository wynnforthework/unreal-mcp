"""
Agent Orchestration MCP Server

Main orchestration server that manages the agent workflow using MCP.
Provides tools for natural language UI generation with progress tracking,
error recovery, and result aggregation.
"""

import asyncio
import json
import logging
from typing import Dict, Any, List, Optional
from mcp.server.fastmcp import FastMCP
from mcp.types import Tool

from orchestrator import AgentOrchestrator, WorkflowConfig, WorkflowResult
from tools.figma_ui_tool import FigmaUITool
from tools.ue_python_tool import UEPythonTool


# Initialize FastMCP server
mcp = FastMCP("agentOrchestrationMCP")

# Global orchestrator instance
orchestrator: Optional[AgentOrchestrator] = None
figma_tool: Optional[FigmaUITool] = None
ue_tool: Optional[UEPythonTool] = None


async def initialize_tools():
    """Initialize the orchestrator and tools"""
    global orchestrator, figma_tool, ue_tool
    
    if orchestrator is None:
        # Initialize orchestrator with default config
        config = WorkflowConfig(
            enable_rollback=True,
            max_retries_per_step=2,
            timeout_per_step=300.0,
            ue_tcp_host="127.0.0.1",
            ue_tcp_port=55557
        )
        orchestrator = AgentOrchestrator(config)
    
    if figma_tool is None:
        figma_tool = FigmaUITool()
    
    if ue_tool is None:
        ue_tool = UEPythonTool()
        await ue_tool.start()


@mcp.tool()
async def generate_ui_from_text(
    natural_language_request: str,
    context: Optional[Dict[str, Any]] = None,
    cpp_header_path: Optional[str] = None
) -> Dict[str, Any]:
    """
    Generate a UMG Widget Blueprint from natural language description.
    
    This tool orchestrates the complete workflow:
    1. Parse natural language requirements
    2. Translate to UMG-compatible design
    3. Execute widget creation in UE
    4. Validate C++ bindings
    5. Generate final report
    
    Args:
        natural_language_request: Natural language description of the UI
        context: Optional context information (project type, style preferences, etc.)
        cpp_header_path: Optional path to C++ header file for binding validation
    
    Returns:
        Complete workflow result with generated widget and validation report
    """
    
    await initialize_tools()
    
    try:
        # Update orchestrator config if cpp_header_path provided
        if cpp_header_path:
            orchestrator.config.cpp_header_path = cpp_header_path
        
        # Execute the complete workflow
        result = await orchestrator.execute_workflow(
            natural_language_request=natural_language_request,
            context=context
        )
        
        return {
            "success": result.status == "success",
            "workflow_id": result.workflow_id,
            "status": result.status,
            "execution_time": result.execution_time,
            "ui_specification": result.ui_specification,
            "umg_definition": result.umg_definition,
            "execution_result": result.execution_result,
            "validation_report": result.validation_report,
            "final_report": result.final_report,
            "step_results": result.step_results,
            "errors": result.errors,
            "warnings": result.warnings
        }
        
    except Exception as e:
        return {
            "success": False,
            "error": str(e),
            "workflow_id": None,
            "status": "failed"
        }


@mcp.tool()
async def generate_ui_from_figma(
    figma_url: str,
    cpp_header_path: Optional[str] = None,
    figma_token: Optional[str] = None
) -> Dict[str, Any]:
    """
    Generate a UMG Widget Blueprint from a Figma design.
    
    Args:
        figma_url: Figma file or frame URL
        cpp_header_path: Optional path to C++ header file for binding validation
        figma_token: Optional Figma API token (if not configured)
    
    Returns:
        Complete workflow result with generated widget and validation report
    """
    
    await initialize_tools()
    
    try:
        # Configure Figma token if provided
        if figma_token:
            figma_tool.figma_token = figma_token
        
        # Fetch design from Figma
        async with figma_tool:
            ui_spec = await figma_tool.fetch_from_url(figma_url, "figma")
        
        # Convert UI spec to natural language for the orchestrator
        natural_language = f"Create a widget based on Figma design: {ui_spec.get('description', 'Imported design')}"
        
        # Update orchestrator config
        if cpp_header_path:
            orchestrator.config.cpp_header_path = cpp_header_path
        
        # Execute workflow with pre-parsed UI specification
        result = await orchestrator.execute_workflow(
            natural_language_request=natural_language,
            context={"source": "figma", "ui_specification": ui_spec}
        )
        
        return {
            "success": result.status == "success",
            "workflow_id": result.workflow_id,
            "status": result.status,
            "execution_time": result.execution_time,
            "figma_design": ui_spec,
            "umg_definition": result.umg_definition,
            "execution_result": result.execution_result,
            "validation_report": result.validation_report,
            "final_report": result.final_report,
            "errors": result.errors,
            "warnings": result.warnings
        }
        
    except Exception as e:
        return {
            "success": False,
            "error": str(e),
            "workflow_id": None,
            "status": "failed"
        }


@mcp.tool()
async def get_workflow_progress(workflow_id: Optional[str] = None) -> Dict[str, Any]:
    """
    Get progress information for a workflow.
    
    Args:
        workflow_id: Optional workflow ID (if None, returns current workflow progress)
    
    Returns:
        Workflow progress information
    """
    
    await initialize_tools()
    
    try:
        progress = orchestrator.get_workflow_progress()
        
        if progress:
            return {
                "success": True,
                "progress": progress
            }
        else:
            return {
                "success": False,
                "error": "No active workflow found"
            }
            
    except Exception as e:
        return {
            "success": False,
            "error": str(e)
        }


@mcp.tool()
async def cancel_workflow() -> Dict[str, Any]:
    """
    Cancel the current workflow.
    
    Returns:
        Cancellation result
    """
    
    await initialize_tools()
    
    try:
        success = await orchestrator.cancel_workflow()
        
        return {
            "success": success,
            "message": "Workflow cancelled" if success else "No active workflow to cancel"
        }
        
    except Exception as e:
        return {
            "success": False,
            "error": str(e)
        }


@mcp.tool()
async def execute_ue_command(
    command_type: str,
    params: Dict[str, Any],
    timeout: Optional[float] = None
) -> Dict[str, Any]:
    """
    Execute a single UE command directly.
    
    Args:
        command_type: Type of UE command to execute
        params: Command parameters
        timeout: Optional timeout in seconds
    
    Returns:
        Command execution result
    """
    
    await initialize_tools()
    
    try:
        result = await ue_tool.execute_command(command_type, params, timeout)
        
        return {
            "success": result.success,
            "command_id": result.command_id,
            "data": result.data,
            "error": result.error,
            "execution_time": result.execution_time,
            "retry_count": result.retry_count
        }
        
    except Exception as e:
        return {
            "success": False,
            "error": str(e)
        }


@mcp.tool()
async def execute_ue_batch(
    commands: List[Dict[str, Any]],
    parallel: bool = False
) -> Dict[str, Any]:
    """
    Execute multiple UE commands in batch.
    
    Args:
        commands: List of commands to execute
        parallel: Whether to execute commands in parallel
    
    Returns:
        Batch execution results
    """
    
    await initialize_tools()
    
    try:
        results = await ue_tool.execute_batch(commands, parallel)
        
        return {
            "success": all(r.success for r in results),
            "results": [
                {
                    "command_id": r.command_id,
                    "success": r.success,
                    "data": r.data,
                    "error": r.error,
                    "execution_time": r.execution_time
                }
                for r in results
            ],
            "total_commands": len(results),
            "successful_commands": sum(1 for r in results if r.success),
            "failed_commands": sum(1 for r in results if not r.success)
        }
        
    except Exception as e:
        return {
            "success": False,
            "error": str(e)
        }


@mcp.tool()
async def get_performance_stats() -> Dict[str, Any]:
    """
    Get performance statistics for the UE tool.
    
    Returns:
        Performance statistics
    """
    
    await initialize_tools()
    
    try:
        stats = ue_tool.get_performance_stats()
        
        return {
            "success": True,
            "stats": stats
        }
        
    except Exception as e:
        return {
            "success": False,
            "error": str(e)
        }


if __name__ == "__main__":
    # Setup logging
    logging.basicConfig(level=logging.INFO)
    
    # Run the MCP server
    mcp.run(transport='stdio')
