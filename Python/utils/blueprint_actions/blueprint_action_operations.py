"""
Blueprint Action Operations for Unreal MCP.

This module provides utility functions for discovering Blueprint actions using the FBlueprintActionDatabase.
"""

import logging
from typing import Dict, Any
from mcp.server.fastmcp import Context
from utils.unreal_connection_utils import send_unreal_command

# Get logger
logger = logging.getLogger("UnrealMCP")

def get_actions_for_pin(
    ctx: Context,
    pin_type: str,
    pin_subcategory: str = ""
) -> Dict[str, Any]:
    """Implementation for getting actions for a specific pin type."""
    params = {
        "pin_type": pin_type,
        "pin_subcategory": pin_subcategory
    }
    return send_unreal_command("get_actions_for_pin", params)

def get_actions_for_class(
    ctx: Context,
    class_name: str
) -> Dict[str, Any]:
    """Implementation for getting actions for a specific class."""
    params = {
        "class_name": class_name
    }
    return send_unreal_command("get_actions_for_class", params)

def get_actions_for_class_hierarchy(
    ctx: Context,
    class_name: str
) -> Dict[str, Any]:
    """Implementation for getting actions for a class hierarchy."""
    params = {
        "class_name": class_name
    }
    return send_unreal_command("get_actions_for_class_hierarchy", params)

def get_node_pin_info(
    ctx: Context,
    node_name: str,
    pin_name: str
) -> Dict[str, Any]:
    """Implementation for getting specific node pin information."""
    params = {
        "node_name": node_name,
        "pin_name": pin_name
    }
    return send_unreal_command("get_node_pin_info", params) 