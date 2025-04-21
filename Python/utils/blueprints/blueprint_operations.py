"""
Blueprint Operations for Unreal MCP.

This module provides utility functions for working with Blueprints in Unreal Engine.
"""

import logging
from typing import Dict, List, Any, Tuple
from mcp.server.fastmcp import Context
from utils.unreal_connection_utils import send_unreal_command

# Get logger
logger = logging.getLogger("UnrealMCP")

def list_all_blueprints(ctx: Context) -> List[Dict[str, Any]]:
    """Implementation for getting a list of all blueprints in the project."""
    return send_unreal_command("list_all_blueprints", {})

def search_blueprints(ctx: Context, query: str) -> List[Dict[str, Any]]:
    """Implementation for searching blueprints matching a query string."""
    params = {"query": query}
    return send_unreal_command("search_blueprints", params)

def get_blueprint_details(ctx: Context, blueprint_path: str) -> Dict[str, Any]:
    """Implementation for getting detailed information about a specific blueprint."""
    params = {"blueprint_path": blueprint_path}
    return send_unreal_command("get_blueprint_details", params)

def create_blueprint_from_actor(
    ctx: Context,
    actor_name: str,
    blueprint_name: str,
    folder_path: str = "/Game/Blueprints"
) -> Dict[str, Any]:
    """Implementation for creating a new blueprint from an existing actor."""
    params = {
        "actor_name": actor_name,
        "blueprint_name": blueprint_name,
        "folder_path": folder_path
    }
    return send_unreal_command("create_blueprint_from_actor", params)

def create_blank_blueprint(
    ctx: Context,
    blueprint_name: str,
    parent_class: str = "Actor",
    folder_path: str = "/Game/Blueprints"
) -> Dict[str, Any]:
    """Implementation for creating a new blank blueprint."""
    params = {
        "blueprint_name": blueprint_name,
        "parent_class": parent_class,
        "folder_path": folder_path
    }
    return send_unreal_command("create_blank_blueprint", params)

def compile_blueprint(ctx: Context, blueprint_path: str) -> Dict[str, Any]:
    """Implementation for compiling a blueprint."""
    params = {"blueprint_path": blueprint_path}
    return send_unreal_command("compile_blueprint", params)

def save_blueprint(ctx: Context, blueprint_path: str) -> Dict[str, Any]:
    """Implementation for saving a blueprint to disk."""
    params = {"blueprint_path": blueprint_path}
    return send_unreal_command("save_blueprint", params)

def get_blueprint_variables(ctx: Context, blueprint_path: str) -> List[Dict[str, Any]]:
    """Implementation for getting a list of variables defined in a blueprint."""
    params = {"blueprint_path": blueprint_path}
    return send_unreal_command("get_blueprint_variables", params)

def add_blueprint_variable(
    ctx: Context,
    blueprint_path: str,
    var_name: str,
    var_type: str,
    default_value: Any = None,
    is_instance_editable: bool = True,
    is_blueprint_read_only: bool = False,
    category: str = "Default"
) -> Dict[str, Any]:
    """Implementation for adding a new variable to a blueprint."""
    params = {
        "blueprint_path": blueprint_path,
        "var_name": var_name,
        "var_type": var_type,
        "is_instance_editable": is_instance_editable,
        "is_blueprint_read_only": is_blueprint_read_only,
        "category": category
    }
    
    # Only include default value if provided
    if default_value is not None:
        params["default_value"] = default_value
    
    return send_unreal_command("add_blueprint_variable", params)

def get_blueprint_functions(ctx: Context, blueprint_path: str) -> List[Dict[str, Any]]:
    """Implementation for getting a list of functions defined in a blueprint."""
    params = {"blueprint_path": blueprint_path}
    return send_unreal_command("get_blueprint_functions", params)

def add_blueprint_function(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    inputs: List[Dict[str, Any]] = None,
    outputs: List[Dict[str, Any]] = None,
    pure: bool = False,
    static: bool = False,
    category: str = "Default"
) -> Dict[str, Any]:
    """Implementation for adding a new function to a blueprint."""
    if inputs is None:
        inputs = []
    if outputs is None:
        outputs = []
    
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "inputs": inputs,
        "outputs": outputs,
        "pure": pure,
        "static": static,
        "category": category
    }
    
    return send_unreal_command("add_blueprint_function", params)

def add_graph_node(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    node_type: str,
    node_name: str,
    x: float,
    y: float,
    inputs: Dict[str, Any] = None,
    outputs: Dict[str, Any] = None
) -> Dict[str, Any]:
    """Implementation for adding a node to a blueprint graph."""
    if inputs is None:
        inputs = {}
    if outputs is None:
        outputs = {}
    
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "node_type": node_type,
        "node_name": node_name,
        "x": x,
        "y": y,
        "inputs": inputs,
        "outputs": outputs
    }
    
    return send_unreal_command("add_graph_node", params)

def connect_graph_nodes(
    ctx: Context,
    blueprint_path: str,
    function_name: str,
    source_node_name: str,
    source_pin_name: str,
    target_node_name: str,
    target_pin_name: str
) -> Dict[str, Any]:
    """Implementation for connecting two nodes in a blueprint graph."""
    params = {
        "blueprint_path": blueprint_path,
        "function_name": function_name,
        "source_node_name": source_node_name,
        "source_pin_name": source_pin_name,
        "target_node_name": target_node_name,
        "target_pin_name": target_pin_name
    }
    
    return send_unreal_command("connect_graph_nodes", params)