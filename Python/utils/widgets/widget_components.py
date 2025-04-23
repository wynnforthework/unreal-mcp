"""
Widget Components for Unreal MCP.

This module provides utilities for working with UMG Widget Components in Unreal Engine.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import Context
from utils.unreal_connection_utils import send_unreal_command

# Get logger
logger = logging.getLogger("UnrealMCP")

def create_widget_blueprint(
    ctx: Context,
    widget_name: str,
    parent_class: str = "UserWidget",
    path: str = "/Game/Widgets"
) -> Dict[str, Any]:
    """Implementation for creating a new UMG Widget Blueprint."""
    params = {
        "name": widget_name,
        "parent_class": parent_class,
        "path": path
    }
    
    return send_unreal_command("create_umg_widget_blueprint", params)

def add_text_block(
    ctx: Context,
    widget_name: str,
    text_block_name: str,
    text: str = "",
    position: List[float] = [0.0, 0.0],
    size: List[float] = [200.0, 50.0],
    font_size: int = 12,
    color: List[float] = [1.0, 1.0, 1.0, 1.0]
) -> Dict[str, Any]:
    """Implementation for adding a Text Block widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,  # Name of the Widget Blueprint asset
        "widget_name": text_block_name, # Name for the Text Block widget being created
        "text": text,
        "position": position,
        "size": size,
        "font_size": font_size,
        "color": color
    }
    
    return send_unreal_command("add_text_block_to_widget", params)

def add_button(
    ctx: Context,
    widget_name: str,
    button_name: str,
    text: str = "",
    position: List[float] = [0.0, 0.0],
    size: List[float] = [200.0, 50.0],
    font_size: int = 12,
    color: List[float] = [1.0, 1.0, 1.0, 1.0],
    background_color: List[float] = [0.1, 0.1, 0.1, 1.0]
) -> Dict[str, Any]:
    """Implementation for adding a Button widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,    # Name of the Widget Blueprint asset
        "widget_name": button_name,       # Name for the Button widget being created
        "text": text,
        "position": position,
        "size": size,
        "font_size": font_size,
        "color": color,
        "background_color": background_color
    }
    
    return send_unreal_command("add_button_to_widget", params)

def add_image(
    ctx: Context,
    widget_name: str, 
    image_name: str,
    brush_asset_path: str = "",
    position: List[float] = [0.0, 0.0],
    size: List[float] = [100.0, 100.0]
) -> Dict[str, Any]:
    """Implementation for adding an Image widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": image_name,
        "brush_asset_path": brush_asset_path,
        "position": position,
        "size": size
    }
    
    return send_unreal_command("add_image_to_widget", params)

def add_check_box(
    ctx: Context,
    widget_name: str, 
    checkbox_name: str,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [40.0, 40.0],
    is_checked: bool = False
) -> Dict[str, Any]:
    """Implementation for adding a CheckBox widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": checkbox_name,
        "position": position,
        "size": size,
        "is_checked": is_checked
    }
    
    return send_unreal_command("add_check_box_to_widget", params)

def bind_event(
    ctx: Context,
    widget_name: str,
    widget_component_name: str,
    event_name: str,
    function_name: str = ""
) -> Dict[str, Any]:
    """Implementation for binding an event on a widget component to a function."""
    # If no function name provided, create one from component and event names
    if not function_name:
        function_name = f"{widget_component_name}_{event_name}"
    
    params = {
        "blueprint_name": widget_name,   # Name of the Widget Blueprint asset
        "widget_name": widget_component_name,  # Name of the component in the widget
        "event_name": event_name,
        "function_name": function_name
    }
    
    return send_unreal_command("bind_widget_event", params)

def add_to_viewport(
    ctx: Context,
    widget_name: str,
    z_order: int = 0
) -> Dict[str, Any]:
    """Implementation for adding a Widget Blueprint instance to the viewport."""
    params = {
        "blueprint_name": widget_name,  # Name of the Widget Blueprint asset
        "z_order": z_order
    }
    
    return send_unreal_command("add_widget_to_viewport", params)

def set_text_binding(
    ctx: Context,
    widget_name: str,
    text_block_name: str,
    binding_property: str,
    binding_type: str = "Text"
) -> Dict[str, Any]:
    """Implementation for setting up a property binding for a Text Block widget."""
    params = {
        "blueprint_name": widget_name,      # Name of the Widget Blueprint asset
        "widget_name": text_block_name,     # Name of the Text Block widget in the blueprint
        "binding_name": binding_property,   # Name of the property to bind to
        "binding_type": binding_type
    }
    
    return send_unreal_command("set_text_block_binding", params)

def add_slider(
    ctx: Context,
    widget_name: str,
    slider_name: str,
    min_value: float = 0.0,
    max_value: float = 1.0,
    value: float = 0.5,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [200.0, 30.0],
    orientation: str = "Horizontal",
    bar_color: List[float] = [0.2, 0.2, 0.8, 1.0],
    handle_color: List[float] = [1.0, 1.0, 1.0, 1.0]
) -> Dict[str, Any]:
    """Implementation for adding a Slider widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": slider_name,
        "min_value": min_value,
        "max_value": max_value,
        "value": value,
        "position": position,
        "size": size,
        "orientation": orientation,
        "bar_color": bar_color,
        "handle_color": handle_color
    }
    
    return send_unreal_command("add_slider_to_widget", params)

def add_progress_bar(
    ctx: Context,
    widget_name: str,
    progress_bar_name: str,
    percent: float = 0.5,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [200.0, 20.0],
    fill_color: List[float] = [0.0, 0.5, 1.0, 1.0],
    background_color: List[float] = [0.1, 0.1, 0.1, 1.0]
) -> Dict[str, Any]:
    """Implementation for adding a ProgressBar widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": progress_bar_name,
        "percent": percent,
        "position": position,
        "size": size,
        "fill_color": fill_color,
        "background_color": background_color
    }
    
    return send_unreal_command("add_progress_bar_to_widget", params)

def add_border(
    ctx: Context,
    widget_name: str,
    border_name: str,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [200.0, 200.0],
    brush_color: List[float] = [0.1, 0.1, 0.1, 1.0],
    brush_thickness: float = 4.0
) -> Dict[str, Any]:
    """Implementation for adding a Border widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": border_name,
        "position": position,
        "size": size,
        "brush_color": brush_color,
        "brush_thickness": brush_thickness
    }
    
    return send_unreal_command("add_border_to_widget", params)

# More container widgets

def add_scroll_box(
    ctx: Context,
    widget_name: str,
    scroll_box_name: str,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [300.0, 200.0],
    orientation: str = "Vertical",
    scroll_bar_visibility: str = "Visible"
) -> Dict[str, Any]:
    """Implementation for adding a ScrollBox widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": scroll_box_name,
        "position": position,
        "size": size,
        "orientation": orientation,
        "scroll_bar_visibility": scroll_bar_visibility
    }
    
    return send_unreal_command("add_scroll_box_to_widget", params)

def add_spacer(
    ctx: Context,
    widget_name: str,
    spacer_name: str,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [100.0, 100.0]
) -> Dict[str, Any]:
    """Implementation for adding a Spacer widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": spacer_name,
        "position": position,
        "size": size
    }
    
    return send_unreal_command("add_spacer_to_widget", params)

def add_widget_switcher(
    ctx: Context,
    widget_name: str,
    switcher_name: str,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [400.0, 300.0],
    active_widget_index: int = 0
) -> Dict[str, Any]:
    """Implementation for adding a Widget Switcher component to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": switcher_name,
        "position": position,
        "size": size,
        "active_widget_index": active_widget_index
    }
    
    return send_unreal_command("add_widget_switcher_to_widget", params)

# Additional specialized widgets

def add_throbber(
    ctx: Context,
    widget_name: str,
    throbber_name: str,
    position: List[float] = [0.0, 0.0],
    size: List[float] = [100.0, 20.0],
    num_pieces: int = 3,
    animate: bool = True
) -> Dict[str, Any]:
    """Implementation for adding a Throbber widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": throbber_name,
        "position": position,
        "size": size,
        "num_pieces": num_pieces,
        "animate": animate
    }
    
    return send_unreal_command("add_throbber_to_widget", params)

def add_expandable_area(
    ctx: Context,
    widget_name: str,
    expandable_area_name: str,
    header_text: str = "Header",
    position: List[float] = [0.0, 0.0],
    size: List[float] = [300.0, 100.0],
    is_expanded: bool = False
) -> Dict[str, Any]:
    """Implementation for adding an Expandable Area widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": expandable_area_name,
        "header_text": header_text,
        "position": position,
        "size": size,
        "is_expanded": is_expanded
    }
    
    return send_unreal_command("add_expandable_area_to_widget", params)

def add_rich_text_block(
    ctx: Context,
    widget_name: str,
    rich_text_name: str,
    text: str = "",
    position: List[float] = [0.0, 0.0],
    size: List[float] = [300.0, 150.0],
    font_size: int = 12,
    default_color: List[float] = [1.0, 1.0, 1.0, 1.0],
    auto_wrap_text: bool = True
) -> Dict[str, Any]:
    """Implementation for adding a Rich Text Block widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": rich_text_name,
        "text": text,
        "position": position,
        "size": size,
        "font_size": font_size,
        "default_color": default_color,
        "auto_wrap_text": auto_wrap_text
    }
    
    return send_unreal_command("add_rich_text_block_to_widget", params)

def add_multi_line_editable_text(
    ctx: Context,
    widget_name: str,
    text_box_name: str,
    hint_text: str = "",
    text: str = "",
    position: List[float] = [0.0, 0.0],
    size: List[float] = [300.0, 150.0],
    allows_multiline: bool = True
) -> Dict[str, Any]:
    """Implementation for adding a Multi-Line Editable Text widget to a UMG Widget Blueprint."""
    params = {
        "blueprint_name": widget_name,
        "widget_name": text_box_name,
        "hint_text": hint_text,
        "text": text,
        "position": position,
        "size": size,
        "allows_multiline": allows_multiline
    }
    
    return send_unreal_command("add_multi_line_editable_text_to_widget", params)

def add_widget_as_child(
    ctx: Context,
    widget_name: str,
    parent_component_name: str,
    child_component_name: str,
    create_parent_if_missing: bool = False,
    parent_component_type: str = "Border",
    parent_position: List[float] = [0.0, 0.0],
    parent_size: List[float] = [300.0, 200.0]
) -> Dict[str, Any]:
    """Implementation for adding a widget component as child to another component."""
    params = {
        "blueprint_name": widget_name,
        "parent_component_name": parent_component_name,
        "child_component_name": child_component_name,
        "create_parent_if_missing": create_parent_if_missing,
        "parent_component_type": parent_component_type,
        "parent_position": parent_position,
        "parent_size": parent_size
    }
    
    return send_unreal_command("add_widget_as_child", params)

def create_widget_component_with_child(
    ctx: Context,
    widget_name: str,
    parent_component_name: str,
    child_component_name: str,
    parent_component_type: str = "Border",
    child_component_type: str = "TextBlock",
    parent_position: List[float] = [0.0, 0.0],
    parent_size: List[float] = [300.0, 200.0],
    child_attributes: Dict[str, Any] = None
) -> Dict[str, Any]:
    """Implementation for creating a new parent widget component with a new child component."""
    if child_attributes is None:
        child_attributes = {}
        
    params = {
        "blueprint_name": widget_name,
        "parent_component_name": parent_component_name,
        "child_component_name": child_component_name,
        "parent_component_type": parent_component_type,
        "child_component_type": child_component_type,
        "parent_position": parent_position,
        "parent_size": parent_size,
        "child_attributes": child_attributes
    }
    
    return send_unreal_command("create_widget_component_with_child", params)

def check_component_exists(
    ctx: Context,
    widget_name: str,
    component_name: str
) -> Dict[str, Any]:
    """Implementation for checking if a component exists in a widget blueprint."""
    params = {
        "blueprint_name": widget_name,
        "component_name": component_name
    }
    
    return send_unreal_command("check_component_exists", params)

def set_widget_component_placement(
    ctx: Context,
    widget_name: str,
    component_name: str,
    position: List[float] = None,
    size: List[float] = None,
    alignment: List[float] = None
) -> Dict[str, Any]:
    """Implementation for changing the placement (position/size) of a widget component.
    
    Args:
        ctx: The current context
        widget_name: Name of the target Widget Blueprint
        component_name: Name of the component to modify
        position: Optional [X, Y] new position in the canvas panel
        size: Optional [Width, Height] new size for the component
        alignment: Optional [X, Y] alignment values (0.0 to 1.0)
        
    Returns:
        Dict containing success status and updated placement information
    """
    params = {
        "widget_name": widget_name,
        "component_name": component_name
    }
    
    # Only add parameters that are actually provided
    if position is not None:
        params["position"] = position
    
    if size is not None:
        params["size"] = size
        
    if alignment is not None:
        params["alignment"] = alignment
    
    return send_unreal_command("set_widget_component_placement", params)

def get_widget_container_dimensions(
    ctx: Context,
    widget_name: str,
    container_name: str = "RootCanvas"
) -> Dict[str, Any]:
    """Implementation for getting the dimensions of a container widget in a UMG Widget Blueprint.
    
    Args:
        ctx: The current context
        widget_name: Name of the target Widget Blueprint
        container_name: Name of the container widget (defaults to "RootCanvas" for the root canvas panel)
        
    Returns:
        Dict containing the container dimensions (width, height) and its position
    """
    params = {
        "widget_name": widget_name,
        "container_name": container_name
    }
    
    return send_unreal_command("get_widget_container_dimensions", params)