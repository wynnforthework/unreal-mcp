"""
UMG Tools for Unreal MCP.

This module provides tools for creating and manipulating UMG Widget Blueprints in Unreal Engine.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import FastMCP, Context
# Use _impl aliases for implementation functions
from utils.widgets.widget_components import (
    create_widget_blueprint as create_widget_blueprint_impl,
    bind_widget_component_event as bind_widget_component_event_impl,
    add_widget_to_viewport as add_widget_to_viewport_impl,
    set_text_block_widget_component_binding as set_text_block_widget_component_binding_impl,
    add_child_widget_component_to_parent as add_child_widget_component_to_parent_impl,
    create_parent_and_child_widget_components as create_parent_and_child_widget_components_impl,
    check_widget_component_exists as check_widget_component_exists_impl,
    set_widget_component_placement as set_widget_component_placement_impl,
    get_widget_container_component_dimensions as get_widget_container_component_dimensions_impl,
    add_widget_component_to_widget as add_widget_component_to_widget_impl,
    set_widget_component_property as set_widget_component_property_impl
)

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_umg_tools(mcp: FastMCP):
    """Register UMG tools with the MCP server."""

    @mcp.tool()
    def create_umg_widget_blueprint(
        ctx: Context,
        widget_name: str,
        parent_class: str = "UserWidget",
        path: str = "/Game/Widgets"
    ) -> Dict[str, object]:
        """
        Create a new UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the widget blueprint to create
            parent_class: Parent class for the widget (default: UserWidget)
            path: Content browser path where the widget should be created
            
        Returns:
            Dict containing success status and widget path
            
        Examples:
            # Create a basic UserWidget blueprint
            create_umg_widget_blueprint(widget_name="MyWidget")
            
            # Create a widget with custom parent class
            create_umg_widget_blueprint(widget_name="MyCustomWidget", parent_class="MyBaseUserWidget")
            
            # Create a widget in a custom folder
            create_umg_widget_blueprint(widget_name="MyWidget", path="/Game/UI/Widgets")
        """
        # Call aliased implementation
        return create_widget_blueprint_impl(ctx, widget_name, parent_class, path)

    @mcp.tool()
    def bind_widget_component_event(
        ctx: Context,
        widget_name: str,
        widget_component_name: str,
        event_name: str,
        function_name: str = ""
    ) -> Dict[str, object]:
        """
        Bind an event on a widget component to a function.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            widget_component_name: Name of the widget component (button, etc.)
            event_name: Name of the event to bind (OnClicked, etc.)
            function_name: Name of the function to create/bind to (defaults to f"{widget_component_name}_{event_name}")
            
        Returns:
            Dict containing success status and binding information
            
        Examples:
            # Bind button click event
            bind_widget_component_event(
                widget_name="LoginScreen",
                widget_component_name="LoginButton",
                event_name="OnClicked"
            )
            
            # Bind with custom function name
            bind_widget_component_event(
                widget_name="MainMenu",
                widget_component_name="QuitButton",
                event_name="OnClicked",
                function_name="ExitApplication"
            )
        """
        # Call aliased implementation
        return bind_widget_component_event_impl(ctx, widget_name, widget_component_name, event_name, function_name)

    @mcp.tool()
    def add_widget_to_viewport(
        ctx: Context,
        widget_name: str,
        z_order: int = 0
    ) -> Dict[str, object]:
        """
        Add a Widget Blueprint instance to the viewport.
        
        Args:
            widget_name: Name of the Widget Blueprint to add
            z_order: Z-order for the widget (higher numbers appear on top)
            
        Returns:
            Dict containing success status and widget instance information
            
        Examples:
            # Add widget to viewport with default z-order
            add_widget_to_viewport(widget_name="MainMenu")
            
            # Add widget with specific z-order (higher number appears on top)
            add_widget_to_viewport(widget_name="NotificationWidget", z_order=10)
        """
        # Call aliased implementation
        return add_widget_to_viewport_impl(ctx, widget_name, z_order)

    @mcp.tool()
    def set_text_block_widget_component_binding(
        ctx: Context,
        widget_name: str,
        text_block_name: str,
        binding_property: str,
        binding_type: str = "Text"
    ) -> Dict[str, object]:
        """
        Set up a property binding for a Text Block widget.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            text_block_name: Name of the Text Block to bind
            binding_property: Name of the property to bind to
            binding_type: Type of binding (Text, Visibility, etc.)
            
        Returns:
            Dict containing success status and binding information
            
        Examples:
            # Set basic text binding
            set_text_block_widget_component_binding(
                widget_name="PlayerHUD",
                text_block_name="ScoreText",
                binding_property="CurrentScore"
            )
            
            # Set binding with specific binding type
            set_text_block_widget_component_binding(
                widget_name="GameUI",
                text_block_name="TimerText",
                binding_property="RemainingTime",
                binding_type="Text"
            )
        """
        # Call aliased implementation
        return set_text_block_widget_component_binding_impl(ctx, widget_name, text_block_name, binding_property, binding_type)

    @mcp.tool()
    def add_child_widget_component_to_parent(
        ctx: Context,
        widget_name: str,
        parent_component_name: str,
        child_component_name: str,
        create_parent_if_missing: bool = False,
        parent_component_type: str = "Border",
        parent_position: List[float] = [0.0, 0.0],
        parent_size: List[float] = [300.0, 200.0]
    ) -> Dict[str, object]:
        """
        Add a widget component as a child to another component.
        
        This function can:
        1. Add an existing component inside an existing component
        2. If the parent doesn't exist, optionally create it and add the child inside
        
        Args:
            widget_name: Name of the target Widget Blueprint
            parent_component_name: Name of the parent component
            child_component_name: Name of the child component to add to the parent
            create_parent_if_missing: Whether to create the parent component if it doesn't exist
            parent_component_type: Type of parent component to create if needed (e.g., "Border", "VerticalBox")
            parent_position: [X, Y] position of the parent component if created
            parent_size: [Width, Height] of the parent component if created
            
        Returns:
            Dict containing success status and component relationship information
            
        Examples:
            # Add an existing text block inside an existing border
            add_child_widget_component_to_parent(
                widget_name="MyWidget",
                parent_component_name="ContentBorder",
                child_component_name="HeaderText"
            )
            
            # Add an existing button to a new vertical box (creates if missing)
            add_child_widget_component_to_parent(
                widget_name="GameMenu",
                parent_component_name="ButtonsContainer",
                child_component_name="StartButton",
                create_parent_if_missing=True,
                parent_component_type="VerticalBox",
                parent_position=[100.0, 100.0],
                parent_size=[300.0, 400.0]
            )
        """
        # Call aliased implementation
        return add_child_widget_component_to_parent_impl(
            ctx, 
            widget_name, 
            parent_component_name, 
            child_component_name, 
            create_parent_if_missing, 
            parent_component_type, 
            parent_position, 
            parent_size
        )

    @mcp.tool()
    def create_parent_and_child_widget_components(
        ctx: Context,
        widget_name: str,
        parent_component_name: str,
        child_component_name: str,
        parent_component_type: str = "Border",
        child_component_type: str = "TextBlock",
        parent_position: List[float] = [0.0, 0.0],
        parent_size: List[float] = [300.0, 200.0],
        child_attributes: Dict[str, object] = None
    ) -> Dict[str, object]:
        """
        Create a new parent widget component with a new child component.
        
        This function creates both components from scratch:
        1. Creates the parent component
        2. Creates the child component
        3. Adds the child inside the parent
        Note: This function creates exactly one parent and one child component.
              It cannot be used to create multiple nested levels in a single call.

        Args:
            widget_name: Name of the target Widget Blueprint
            parent_component_name: Name for the new parent component
            child_component_name: Name for the new child component
            parent_component_type: Type of parent component to create (e.g., "Border", "VerticalBox")
            child_component_type: Type of child component to create (e.g., "TextBlock", "Button")
            parent_position: [X, Y] position of the parent component
            parent_size: [Width, Height] of the parent component
            child_attributes: Additional attributes for the child component (content, colors, etc.)
            
        Returns:
            Dict containing success status and component creation information
            
        Examples:
            # Create a border with a text block inside
            create_parent_and_child_widget_components(
                widget_name="MyWidget",
                parent_component_name="HeaderBorder",
                child_component_name="TitleText",
                parent_component_type="Border",
                child_component_type="TextBlock",
                parent_position=[50.0, 50.0],
                parent_size=[400.0, 100.0],
                child_attributes={"text": "Welcome to My Game", "font_size": 24}
            )
            
            # Create a scroll box with a vertical box inside
            create_parent_and_child_widget_components(
                widget_name="InventoryScreen",
                parent_component_name="ItemsScrollBox",
                child_component_name="ItemsContainer",
                parent_component_type="ScrollBox",
                child_component_type="VerticalBox",
                parent_position=[100.0, 200.0],
                parent_size=[300.0, 400.0]
            )
        """
        # Call aliased implementation
        return create_parent_and_child_widget_components_impl(
            ctx, 
            widget_name, 
            parent_component_name, 
            child_component_name, 
            parent_component_type, 
            child_component_type, 
            parent_position, 
            parent_size,
            child_attributes
        )

    @mcp.tool()
    def check_widget_component_exists(
        ctx: Context,
        widget_name: str,
        component_name: str
    ) -> Dict[str, object]:
        """
        Check if a component exists in the specified widget blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            component_name: Name of the component to check
            
        Returns:
            Dict containing existence status of the component
            
        Examples:
            # Check if a component exists
            check_widget_component_exists(
                widget_name="MyWidget",
                component_name="HeaderText"
            )
        """
        # Call aliased implementation
        return check_widget_component_exists_impl(ctx, widget_name, component_name)

    @mcp.tool()
    def set_widget_component_placement(
        ctx: Context,
        widget_name: str,
        component_name: str,
        position: List[float] = None,
        size: List[float] = None,
        alignment: List[float] = None
    ) -> Dict[str, object]:
        """
        Change the placement (position/size) of a widget component.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            component_name: Name of the component to modify
            position: Optional [X, Y] new position in the canvas panel
            size: Optional [Width, Height] new size for the component
            alignment: Optional [X, Y] alignment values (0.0 to 1.0)
            
        Returns:
            Dict containing success status and updated placement information
            
        Examples:
            # Change just the position of a component
            set_widget_component_placement(
                widget_name="MainMenu",
                component_name="TitleText",
                position=[350.0, 75.0]
            )
            
            # Change both position and size
            set_widget_component_placement(
                widget_name="HUD",
                component_name="HealthBar",
                position=[50.0, 25.0],
                size=[250.0, 30.0]
            )
            
            # Change alignment (center-align)
            set_widget_component_placement(
                widget_name="Inventory",
                component_name="ItemName",
                alignment=[0.5, 0.5]
            )
        """
        # Call aliased implementation
        return set_widget_component_placement_impl(ctx, widget_name, component_name, position, size, alignment)
        
    @mcp.tool()
    def get_widget_container_component_dimensions(
        ctx: Context,
        widget_name: str,
        container_name: str = "CanvasPanel_0"
    ) -> Dict[str, object]:
        """
        Get the dimensions of a container widget in a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            container_name: Name of the container widget (defaults to "CanvasPanel_0" for the root canvas panel)
            
        Returns:
            Dict containing the container dimensions and position
            
        Examples:
            # Get dimensions of the root canvas
            dimensions = get_widget_container_component_dimensions(
                widget_name="MainMenu"
            )
            
            # Get dimensions of a specific container
            dimensions = get_widget_container_component_dimensions(
                widget_name="InventoryScreen",
                container_name="ItemsContainer"
            )
            
            # Use the dimensions to place a widget in the top-right corner with a 10px margin
            dimensions = get_widget_container_component_dimensions(widget_name="HUD")
            set_widget_component_placement(
                widget_name="HUD",
                component_name="CloseButton",
                position=[dimensions["width"] - 10, 10],
                alignment=[1.0, 0.0]
            )
        """
        # Call aliased implementation
        return get_widget_container_component_dimensions_impl(ctx, widget_name, container_name)

    @mcp.tool()
    def add_widget_component_to_widget(
        ctx: Context,
        widget_name: str,
        component_name: str,
        component_type: str,
        position: List[float] = None,
        size: List[float] = None,
        **kwargs
    ) -> Dict[str, object]:
        """
        Unified function to add any type of widget component to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            component_name: Name to give the new component
            component_type: Type of component to add (e.g., "TextBlock", "Button", etc.)
            position: Optional [X, Y] position in the canvas panel
            size: Optional [Width, Height] of the component
            **kwargs: Additional parameters specific to the component type
            
        Returns:
            Dict containing success status and component properties
            
        Examples:
            # Add a text block
            add_widget_component_to_widget(
                widget_name="MyWidget",
                component_name="HeaderText",
                component_type="TextBlock",
                position=[100, 50],
                size=[200, 50],
                text="Hello World",
                font_size=24
            )
            
            # Add a button
            add_widget_component_to_widget(
                widget_name="MyWidget",
                component_name="SubmitButton",
                component_type="Button",
                position=[100, 100],
                size=[200, 50],
                text="Submit",
                background_color=[0.2, 0.4, 0.8, 1.0]
            )
        """
        # Call aliased implementation
        return add_widget_component_to_widget_impl(ctx, widget_name, component_name, component_type, position, size, **kwargs)

    @mcp.tool()
    def set_widget_component_property(
        ctx: Context,
        widget_name: str,
        component_name: str,
        property_name: str,
        # Explicitly type as str for now, although SetPropertyFromJson handles others
        property_value: str 
    ) -> Dict[str, object]:
        """
        Set a property on a specific component within a UMG Widget Blueprint.

        Args:
            widget_name: Name of the target Widget Blueprint.
            component_name: Name of the component within the widget (e.g., "MyButton", "ScoreTextBlock").
            property_name: Name of the property to set on the component (e.g., "Text", "Visibility", "ToolTipText").
            property_value: The value to set the property to. Type should match the property's expected type.

        Returns:
            Dict containing success status.

        Examples:
            # Set the text of a TextBlock
            set_widget_component_property(
                widget_name="PlayerHUD", 
                component_name="ScoreText", 
                property_name="Text", 
                property_value="Score: 100"
            )

            # Change the visibility of a Border
            set_widget_component_property(
                widget_name="MainMenu",
                component_name="LoginErrorBorder",
                property_name="Visibility",
                property_value="Hidden" # Common visibility values: Visible, Hidden, Collapsed, HitTestInvisible
            )

            # Set the ToolTip text for a Button
            set_widget_component_property(
                widget_name="OptionsMenu",
                component_name="ApplyButton",
                property_name="ToolTipText",
                property_value="Apply graphics settings"
            )
        """
        # Call aliased implementation
        return set_widget_component_property_impl(ctx, widget_name, component_name, property_name, property_value)

logger.info("UMG tools registered successfully")

# Moved outside the function
logger.info("UMG tools module loaded")