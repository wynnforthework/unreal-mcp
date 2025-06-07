"""
Blueprint Action Tools - Dynamically discover available Blueprint actions

This module provides tools for discovering Blueprint actions using the FBlueprintActionDatabase,
allowing dynamic exploration of what nodes and functions are available for different pin types,
classes, and class hierarchies.
"""

import logging
from typing import Dict, Any
from mcp.server.fastmcp import FastMCP, Context
from utils.blueprint_actions.blueprint_action_operations import (
    get_actions_for_pin as get_actions_for_pin_impl,
    get_actions_for_class as get_actions_for_class_impl,
    get_actions_for_class_hierarchy as get_actions_for_class_hierarchy_impl,
    get_node_pin_info as get_node_pin_info_impl
)

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_blueprint_action_tools(mcp: FastMCP):
    """Register blueprint action tools with the MCP server."""

    @mcp.tool()
    def get_actions_for_pin(
        ctx: Context,
        pin_type: str, 
        pin_subcategory: str = ""
    ) -> Dict[str, Any]:
        """
        Get all available Blueprint actions for a specific pin type.
        
        This tool uses Unreal's FBlueprintActionDatabase to dynamically discover what nodes/functions
        are available when connecting to a specific pin type, similar to what you see in the context
        menu when dragging from a pin in the Blueprint editor.
        
        Args:
            pin_type: The type of pin (object, int, float, bool, string, struct, etc.)
            pin_subcategory: The subcategory/class name for object pins (e.g., "PlayerController", "Pawn")
        
        Returns:
            Dict containing:
                - success: Boolean indicating if the operation succeeded
                - actions: List of available actions with title, tooltip, category, keywords
                - pin_type: The pin type that was queried
                - pin_subcategory: The pin subcategory that was queried
                - action_count: Number of actions found
                - message: Status message
        
        Examples:
            # Get actions for a PlayerController object pin
            get_actions_for_pin(pin_type="object", pin_subcategory="PlayerController")
            
            # Get actions for a float pin
            get_actions_for_pin(pin_type="float")
            
            # Get actions for a Vector struct pin
            get_actions_for_pin(pin_type="struct", pin_subcategory="Vector")
        """
        return get_actions_for_pin_impl(ctx, pin_type, pin_subcategory)

    @mcp.tool()
    def get_actions_for_class(
        ctx: Context,
        class_name: str
    ) -> Dict[str, Any]:
        """
        Get all available Blueprint actions for a specific class.
        
        This tool uses Unreal's FBlueprintActionDatabase to discover what functions and nodes
        are available for a specific class. This includes all the BlueprintCallable functions,
        events, and other actions that can be used with instances of this class.
        
        Args:
            class_name: Name or path of the class to get actions for (e.g., "PlayerController", "Pawn", "Actor")
        
        Returns:
            Dict containing:
                - success: Boolean indicating if the operation succeeded
                - actions: List of available actions with title, tooltip, category, keywords
                - class_name: The class name that was queried
                - action_count: Number of actions found
                - message: Status message
        
        Examples:
            # Get actions for PlayerController class
            get_actions_for_class(class_name="PlayerController")
            
            # Get actions for Actor class
            get_actions_for_class(class_name="Actor")
            
            # Get actions using full path
            get_actions_for_class(class_name="/Script/Engine.Pawn")
        """
        return get_actions_for_class_impl(ctx, class_name)

    @mcp.tool()
    def get_actions_for_class_hierarchy(
        ctx: Context,
        class_name: str
    ) -> Dict[str, Any]:
        """
        Get all available Blueprint actions for a class and its entire inheritance hierarchy.
        
        This tool uses Unreal's FBlueprintActionDatabase to discover what functions and nodes
        are available for a class and all of its parent classes. This gives you the complete
        picture of all available actions including inherited functionality.
        
        Args:
            class_name: Name or path of the class to get actions for (e.g., "PlayerController", "Pawn", "Actor")
        
        Returns:
            Dict containing:
                - success: Boolean indicating if the operation succeeded
                - actions: List of available actions with title, tooltip, category, keywords
                - class_hierarchy: List of classes in the inheritance chain
                - category_counts: Count of actions per category
                - class_name: The class name that was queried
                - action_count: Number of actions found
                - message: Status message
        
        Examples:
            # Get actions for PlayerController and its parent classes
            get_actions_for_class_hierarchy(class_name="PlayerController")
            
            # Get comprehensive actions for Pawn class hierarchy
            get_actions_for_class_hierarchy(class_name="Pawn")
            
            # This will include actions from Actor, Object, etc.
            get_actions_for_class_hierarchy(class_name="StaticMeshActor")
        """
        return get_actions_for_class_hierarchy_impl(ctx, class_name)

    @mcp.tool()
    def get_node_pin_info(
        ctx: Context,
        node_name: str, 
        pin_name: str
    ) -> Dict[str, Any]:
        """
        Get specific information about a Blueprint node's pin including expected types.

        This tool provides detailed information about what a specific pin on a specific node
        expects or outputs, including the data type, whether it's required, input/output direction,
        and a description of its purpose.

        Args:
            node_name: Name of the Blueprint node (e.g., "Create Widget", "Get Controller", "Cast to PlayerController")
            pin_name: Name of the specific pin (e.g., "Owning Player", "Class", "Return Value", "Target")

        Returns:
            Dict containing:
                - success: Boolean indicating if the operation succeeded
                - node_name: The node name that was queried
                - pin_name: The pin name that was queried
                - pin_info: Detailed information about the pin including:
                    - pin_type: Type category (object, class, exec, etc.)
                    - expected_type: Specific type expected (PlayerController, Class<UserWidget>, etc.)
                    - description: Description of the pin's purpose
                    - is_required: Whether the pin must be connected
                    - is_input: Whether it's an input (true) or output (false) pin
                - message: Status message
                - available_pins: List of available pins if the node is known but pin is not found

        Examples:
            # Find out what the "Owning Player" pin expects on Create Widget
            get_node_pin_info(node_name="Create Widget", pin_name="Owning Player")
            
            # Check what Get Controller returns
            get_node_pin_info(node_name="Get Controller", pin_name="Return Value")
            
            # Understand Cast to PlayerController inputs
            get_node_pin_info(node_name="Cast to PlayerController", pin_name="Object")
        """
        return get_node_pin_info_impl(ctx, node_name, pin_name) 