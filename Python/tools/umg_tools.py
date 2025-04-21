"""
UMG Tools for Unreal MCP.

This module provides tools for creating and manipulating UMG Widget Blueprints in Unreal Engine.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import FastMCP, Context

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
    ) -> Dict[str, Any]:
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
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "name": widget_name,
                "parent_class": parent_class,
                "path": path
            }
            
            logger.info(f"Creating UMG Widget Blueprint with params: {params}")
            response = unreal.send_command("create_umg_widget_blueprint", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Create UMG Widget Blueprint response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error creating UMG Widget Blueprint: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_text_block_to_widget(
        ctx: Context,
        widget_name: str,
        text_block_name: str,
        text: str = "",
        position: List[float] = [0.0, 0.0],
        size: List[float] = [200.0, 50.0],
        font_size: int = 12,
        color: List[float] = [1.0, 1.0, 1.0, 1.0]
    ) -> Dict[str, Any]:
        """
        Add a Text Block widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            text_block_name: Name to give the new Text Block
            text: Initial text content
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the text block
            font_size: Font size in points
            color: [R, G, B, A] color values (0.0 to 1.0)
            
        Returns:
            Dict containing success status and text block properties
            
        Examples:
            # Add a basic text block
            add_text_block_to_widget(widget_name="MyWidget", text_block_name="HeaderText", text="Hello World")
            
            # Add styled text block
            add_text_block_to_widget(
                widget_name="MainMenu", 
                text_block_name="TitleText",
                text="MAIN MENU",
                position=[400.0, 100.0],
                size=[600.0, 80.0],
                font_size=48,
                color=[1.0, 0.8, 0.2, 1.0]  # Gold color
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,  # Name of the Widget Blueprint asset
                "widget_name": text_block_name, # Name for the Text Block widget being created
                "text": text,
                "position": position,
                "size": size,
                "font_size": font_size,
                "color": color
            }
            
            logger.info(f"Adding Text Block to widget with params: {params}")
            response = unreal.send_command("add_text_block_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Text Block response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Text Block to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_button_to_widget(
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
        """
        Add a Button widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            button_name: Name to give the new Button
            text: Text to display on the button
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the button
            font_size: Font size for button text
            color: [R, G, B, A] text color values (0.0 to 1.0)
            background_color: [R, G, B, A] button background color values (0.0 to 1.0)
            
        Returns:
            Dict containing success status and button properties
            
        Examples:
            # Add a basic button
            add_button_to_widget(widget_name="MyWidget", button_name="SubmitButton", text="Submit")
            
            # Add styled button
            add_button_to_widget(
                widget_name="LoginScreen", 
                button_name="LoginButton",
                text="LOGIN",
                position=[400.0, 300.0],
                size=[300.0, 60.0],
                font_size=18,
                color=[1.0, 1.0, 1.0, 1.0],
                background_color=[0.2, 0.4, 0.8, 1.0]  # Blue color
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
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
            
            logger.info(f"Adding Button to widget with params: {params}")
            response = unreal.send_command("add_button_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Button response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Button to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_image_to_widget(
        ctx: Context,
        widget_name: str, 
        image_name: str,
        brush_asset_path: str = "",
        position: List[float] = [0.0, 0.0],
        size: List[float] = [100.0, 100.0]
    ) -> Dict[str, Any]:
        """
        Add an Image widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            image_name: Name to give the new Image widget
            brush_asset_path: Optional path to the image asset to use
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the image
            
        Returns:
            Dict containing success status and image properties
            
        Examples:
            # Add a basic image
            add_image_to_widget(
                widget_name="MainMenu", 
                image_name="BackgroundImage"
            )
            
            # Add an image with a specific texture
            add_image_to_widget(
                widget_name="HUD", 
                image_name="WeaponIcon",
                brush_asset_path="/Game/UI/Textures/WeaponIcons/Rifle",
                position=[50.0, 50.0],
                size=[64.0, 64.0]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": image_name,
                "brush_asset_path": brush_asset_path,
                "position": position,
                "size": size
            }
            
            logger.info(f"Adding Image to widget with params: {params}")
            response = unreal.send_command("add_image_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Image response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Image to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_check_box_to_widget(
        ctx: Context,
        widget_name: str, 
        checkbox_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [40.0, 40.0],
        is_checked: bool = False
    ) -> Dict[str, Any]:
        """
        Add a CheckBox widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            checkbox_name: Name to give the new CheckBox widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the checkbox
            is_checked: Initial checked state of the checkbox
            
        Returns:
            Dict containing success status and checkbox properties
            
        Examples:
            # Add an unchecked checkbox
            add_check_box_to_widget(
                widget_name="SettingsMenu", 
                checkbox_name="FullscreenToggle"
            )
            
            # Add a checked checkbox at specific position
            add_check_box_to_widget(
                widget_name="OptionsPanel", 
                checkbox_name="AutosaveEnabled",
                position=[150.0, 200.0],
                size=[30.0, 30.0],
                is_checked=True
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": checkbox_name,
                "position": position,
                "size": size,
                "is_checked": is_checked
            }
            
            logger.info(f"Adding CheckBox to widget with params: {params}")
            response = unreal.send_command("add_check_box_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add CheckBox response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding CheckBox to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def bind_widget_event(
        ctx: Context,
        widget_name: str,
        widget_component_name: str,
        event_name: str,
        function_name: str = ""
    ) -> Dict[str, Any]:
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
            bind_widget_event(
                widget_name="LoginScreen",
                widget_component_name="LoginButton",
                event_name="OnClicked"
            )
            
            # Bind with custom function name
            bind_widget_event(
                widget_name="MainMenu",
                widget_component_name="QuitButton",
                event_name="OnClicked",
                function_name="ExitApplication"
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            # If no function name provided, create one from component and event names
            if not function_name:
                function_name = f"{widget_component_name}_{event_name}"
            
            params = {
                "blueprint_name": widget_name,   # Name of the Widget Blueprint asset
                "widget_name": widget_component_name,  # Name of the component in the widget
                "event_name": event_name,
                "function_name": function_name
            }
            
            logger.info(f"Binding widget event with params: {params}")
            response = unreal.send_command("bind_widget_event", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Bind widget event response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error binding widget event: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_widget_to_viewport(
        ctx: Context,
        widget_name: str,
        z_order: int = 0
    ) -> Dict[str, Any]:
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
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,  # Name of the Widget Blueprint asset
                "z_order": z_order
            }
            
            logger.info(f"Adding widget to viewport with params: {params}")
            response = unreal.send_command("add_widget_to_viewport", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add widget to viewport response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding widget to viewport: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def set_text_block_binding(
        ctx: Context,
        widget_name: str,
        text_block_name: str,
        binding_property: str,
        binding_type: str = "Text"
    ) -> Dict[str, Any]:
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
            set_text_block_binding(
                widget_name="PlayerHUD",
                text_block_name="ScoreText",
                binding_property="CurrentScore"
            )
            
            # Set binding with specific binding type
            set_text_block_binding(
                widget_name="GameUI",
                text_block_name="TimerText",
                binding_property="RemainingTime",
                binding_type="Text"
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,      # Name of the Widget Blueprint asset
                "widget_name": text_block_name,     # Name of the Text Block widget in the blueprint
                "binding_name": binding_property,   # Name of the property to bind to
                "binding_type": binding_type
            }
            
            logger.info(f"Setting text block binding with params: {params}")
            response = unreal.send_command("set_text_block_binding", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Set text block binding response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error setting text block binding: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_slider_to_widget(
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
        """
        Add a Slider widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            slider_name: Name to give the new Slider
            min_value: Minimum value of the slider
            max_value: Maximum value of the slider
            value: Initial value of the slider
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the slider
            orientation: Orientation of the slider ("Horizontal" or "Vertical")
            bar_color: [R, G, B, A] color values for the slider bar (0.0 to 1.0)
            handle_color: [R, G, B, A] color values for the slider handle (0.0 to 1.0)
            
        Returns:
            Dict containing success status and slider properties
            
        Examples:
            # Add a basic horizontal slider
            add_slider_to_widget(widget_name="SettingsMenu", slider_name="VolumeSlider")
            
            # Add custom vertical slider
            add_slider_to_widget(
                widget_name="VideoSettings",
                slider_name="BrightnessSlider",
                min_value=0.1,
                max_value=2.0,
                value=1.0,
                position=[50.0, 200.0],
                size=[30.0, 150.0],
                orientation="Vertical",
                bar_color=[0.5, 0.5, 0.5, 1.0],
                handle_color=[0.8, 0.8, 0.8, 1.0]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
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
            
            logger.info(f"Adding Slider to widget with params: {params}")
            response = unreal.send_command("add_slider_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Slider response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Slider to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_progress_bar_to_widget(
        ctx: Context,
        widget_name: str,
        progress_bar_name: str,
        percent: float = 0.5,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [200.0, 20.0],
        fill_color: List[float] = [0.0, 0.5, 1.0, 1.0],
        background_color: List[float] = [0.1, 0.1, 0.1, 1.0]
    ) -> Dict[str, Any]:
        """
        Add a ProgressBar widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            progress_bar_name: Name to give the new ProgressBar
            percent: Initial fill percentage (0.0 to 1.0)
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the progress bar
            fill_color: [R, G, B, A] color values for the fill (0.0 to 1.0)
            background_color: [R, G, B, A] color values for the background (0.0 to 1.0)
            
        Returns:
            Dict containing success status and progress bar properties
            
        Examples:
            # Add a basic progress bar
            add_progress_bar_to_widget(widget_name="LoadingScreen", progress_bar_name="LoadingBar")
            
            # Add custom health bar
            add_progress_bar_to_widget(
                widget_name="PlayerHUD",
                progress_bar_name="HealthBar",
                percent=1.0,
                position=[50.0, 50.0],
                size=[300.0, 25.0],
                fill_color=[0.9, 0.2, 0.2, 1.0],  # Red fill
                background_color=[0.2, 0.0, 0.0, 0.8]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": progress_bar_name,
                "percent": percent,
                "position": position,
                "size": size,
                "fill_color": fill_color,
                "background_color": background_color
            }
            
            logger.info(f"Adding ProgressBar to widget with params: {params}")
            response = unreal.send_command("add_progress_bar_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add ProgressBar response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding ProgressBar to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_border_to_widget(
        ctx: Context,
        widget_name: str,
        border_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [200.0, 200.0],
        brush_color: List[float] = [0.1, 0.1, 0.1, 1.0],
        brush_thickness: float = 4.0
    ) -> Dict[str, Any]:
        """
        Add a Border widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            border_name: Name to give the new Border widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the border
            brush_color: [R, G, B, A] color values for the border (0.0 to 1.0)
            brush_thickness: Thickness of the border in pixels
            
        Returns:
            Dict containing success status and border properties
            
        Examples:
            # Add a basic border
            add_border_to_widget(widget_name="MyWidget", border_name="ContentBorder")
            
            # Add a styled border
            add_border_to_widget(
                widget_name="DialogBox",
                border_name="DialogBorder",
                position=[100.0, 100.0],
                size=[400.0, 300.0],
                brush_color=[0.8, 0.8, 0.8, 1.0],  # Light gray border
                brush_thickness=2.0
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": border_name,
                "position": position,
                "size": size,
                "brush_color": brush_color,
                "brush_thickness": brush_thickness
            }
            
            logger.info(f"Adding Border to widget with params: {params}")
            response = unreal.send_command("add_border_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Border response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Border to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_scroll_box_to_widget(
        ctx: Context,
        widget_name: str,
        scroll_box_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [300.0, 200.0],
        orientation: str = "Vertical",
        scroll_bar_visibility: str = "Visible"
    ) -> Dict[str, Any]:
        """
        Add a ScrollBox widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            scroll_box_name: Name to give the new ScrollBox widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the scroll box
            orientation: Scroll direction ("Vertical" or "Horizontal")
            scroll_bar_visibility: Visibility of the scroll bar ("Visible", "Hidden", "Auto")
            
        Returns:
            Dict containing success status and scroll box properties
            
        Examples:
            # Add a basic vertical scroll box
            add_scroll_box_to_widget(widget_name="MyWidget", scroll_box_name="ContentScroller")
            
            # Add a horizontal scroll box with custom settings
            add_scroll_box_to_widget(
                widget_name="Gallery",
                scroll_box_name="ImagesScroller",
                position=[100.0, 100.0],
                size=[600.0, 150.0],
                orientation="Horizontal",
                scroll_bar_visibility="Auto"
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": scroll_box_name,
                "position": position,
                "size": size,
                "orientation": orientation,
                "scroll_bar_visibility": scroll_bar_visibility
            }
            
            logger.info(f"Adding ScrollBox to widget with params: {params}")
            response = unreal.send_command("add_scroll_box_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add ScrollBox response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding ScrollBox to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_spacer_to_widget(
        ctx: Context,
        widget_name: str,
        spacer_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [100.0, 100.0]
    ) -> Dict[str, Any]:
        """
        Add a Spacer widget to a UMG Widget Blueprint.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            spacer_name: Name to give the new Spacer widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the spacer
            
        Returns:
            Dict containing success status and spacer properties
            
        Examples:
            # Add a basic spacer
            add_spacer_to_widget(widget_name="MyWidget", spacer_name="ContentSpacer")
            
            # Add a spacer with custom size
            add_spacer_to_widget(
                widget_name="MenuLayout",
                spacer_name="ButtonSpacer",
                position=[50.0, 200.0],
                size=[300.0, 20.0]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": spacer_name,
                "position": position,
                "size": size
            }
            
            logger.info(f"Adding Spacer to widget with params: {params}")
            response = unreal.send_command("add_spacer_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Spacer response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Spacer to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_widget_switcher_to_widget(
        ctx: Context,
        widget_name: str,
        switcher_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [400.0, 300.0],
        active_widget_index: int = 0
    ) -> Dict[str, Any]:
        """
        Add a Widget Switcher component to a UMG Widget Blueprint.
        
        A widget switcher is like a tab control, but without tabs. At most one widget is visible at time.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            switcher_name: Name to give the new Widget Switcher
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the widget switcher
            active_widget_index: Index of the initially active widget
            
        Returns:
            Dict containing success status and widget switcher properties
            
        Examples:
            # Add a basic widget switcher
            add_widget_switcher_to_widget(widget_name="MyWidget", switcher_name="ContentSwitcher")
            
            # Add a widget switcher with specific active page
            add_widget_switcher_to_widget(
                widget_name="GameMenu",
                switcher_name="PageSwitcher",
                position=[100.0, 100.0],
                size=[600.0, 400.0],
                active_widget_index=1
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": switcher_name,
                "position": position,
                "size": size,
                "active_widget_index": active_widget_index
            }
            
            logger.info(f"Adding Widget Switcher to widget with params: {params}")
            response = unreal.send_command("add_widget_switcher_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Widget Switcher response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Widget Switcher to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_throbber_to_widget(
        ctx: Context,
        widget_name: str,
        throbber_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [100.0, 20.0],
        num_pieces: int = 3,
        animate: bool = True
    ) -> Dict[str, Any]:
        """
        Add a Throbber widget to a UMG Widget Blueprint.
        
        A simple throbber widget that shows several dots in a horizontal line that animate.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            throbber_name: Name to give the new Throbber widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the throbber
            num_pieces: Number of dots to display
            animate: Whether the throbber should animate automatically
            
        Returns:
            Dict containing success status and throbber properties
            
        Examples:
            # Add a basic throbber
            add_throbber_to_widget(widget_name="MyWidget", throbber_name="LoadingIndicator")
            
            # Add a customized throbber
            add_throbber_to_widget(
                widget_name="LoadingScreen", 
                throbber_name="LoadingDots",
                position=[400.0, 300.0],
                size=[150.0, 30.0],
                num_pieces=5,
                animate=True
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": throbber_name,
                "position": position,
                "size": size,
                "num_pieces": num_pieces,
                "animate": animate
            }
            
            logger.info(f"Adding Throbber to widget with params: {params}")
            response = unreal.send_command("add_throbber_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Throbber response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Throbber to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_expandable_area_to_widget(
        ctx: Context,
        widget_name: str,
        expandable_area_name: str,
        header_text: str = "Header",
        position: List[float] = [0.0, 0.0],
        size: List[float] = [300.0, 100.0],
        is_expanded: bool = False
    ) -> Dict[str, Any]:
        """
        Add an Expandable Area widget to a UMG Widget Blueprint.
        
        An area that can be expanded or collapsed to show/hide its contents.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            expandable_area_name: Name to give the new Expandable Area widget
            header_text: Text to display in the header of the expandable area
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the expandable area
            is_expanded: Whether the area is initially expanded
            
        Returns:
            Dict containing success status and expandable area properties
            
        Examples:
            # Add a basic expandable area
            add_expandable_area_to_widget(
                widget_name="MyWidget", 
                expandable_area_name="DetailsSection",
                header_text="Additional Details"
            )
            
            # Add a customized expandable area that starts expanded
            add_expandable_area_to_widget(
                widget_name="OptionsPanel",
                expandable_area_name="AdvancedOptions",
                header_text="Advanced Settings",
                position=[50.0, 200.0],
                size=[400.0, 150.0],
                is_expanded=True
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": expandable_area_name,
                "header_text": header_text,
                "position": position,
                "size": size,
                "is_expanded": is_expanded
            }
            
            logger.info(f"Adding Expandable Area to widget with params: {params}")
            response = unreal.send_command("add_expandable_area_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Expandable Area response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Expandable Area to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_menu_anchor_to_widget(
        ctx: Context,
        widget_name: str,
        menu_anchor_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [100.0, 30.0],
        placement: str = "Below"
    ) -> Dict[str, Any]:
        """
        Add a Menu Anchor widget to a UMG Widget Blueprint.
        
        The Menu Anchor allows you to specify a location that a popup menu should be anchored to.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            menu_anchor_name: Name to give the new Menu Anchor widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the menu anchor
            placement: Placement of the popup relative to anchor ("Below", "Above", "Left", "Right", "Center")
            
        Returns:
            Dict containing success status and menu anchor properties
            
        Examples:
            # Add a basic menu anchor
            add_menu_anchor_to_widget(
                widget_name="MyWidget", 
                menu_anchor_name="DropdownAnchor"
            )
            
            # Add a customized menu anchor
            add_menu_anchor_to_widget(
                widget_name="MainUI",
                menu_anchor_name="OptionsMenuAnchor",
                position=[200.0, 50.0],
                size=[120.0, 40.0],
                placement="Right"
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": menu_anchor_name,
                "position": position,
                "size": size,
                "placement": placement
            }
            
            logger.info(f"Adding Menu Anchor to widget with params: {params}")
            response = unreal.send_command("add_menu_anchor_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Menu Anchor response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Menu Anchor to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_rich_text_block_to_widget(
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
        """
        Add a Rich Text Block widget to a UMG Widget Blueprint.
        
        The Rich Text Block allows you to display stylized text with markup.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            rich_text_name: Name to give the new Rich Text Block widget
            text: Initial text content (can include markup)
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the rich text block
            font_size: Base font size in points
            default_color: [R, G, B, A] default color values (0.0 to 1.0)
            auto_wrap_text: Whether text should wrap automatically
            
        Returns:
            Dict containing success status and rich text block properties
            
        Examples:
            # Add a basic rich text block
            add_rich_text_block_to_widget(
                widget_name="MyWidget", 
                rich_text_name="StyledText",
                text="Welcome to <RichText.Bold>My Game</>"
            )
            
            # Add a customized rich text block
            add_rich_text_block_to_widget(
                widget_name="TutorialScreen",
                rich_text_name="InstructionsText",
                text="Press <RichText.Bold>W</> to move <RichText.Italic>forward</>\nPress <RichText.Bold>S</> to move <RichText.Italic>backward</>",
                position=[100.0, 200.0],
                size=[400.0, 200.0],
                font_size=16,
                default_color=[0.9, 0.9, 0.9, 1.0]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
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
            
            logger.info(f"Adding Rich Text Block to widget with params: {params}")
            response = unreal.send_command("add_rich_text_block_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Rich Text Block response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Rich Text Block to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_safe_zone_to_widget(
        ctx: Context,
        widget_name: str,
        safe_zone_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [600.0, 400.0],
        padding: List[float] = [0.0, 0.0, 0.0, 0.0]
    ) -> Dict[str, Any]:
        """
        Add a Safe Zone widget to a UMG Widget Blueprint.
        
        The Safe Zone widget ensures UI elements stay in the visible "safe" area of the screen across different devices.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            safe_zone_name: Name to give the new Safe Zone widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the safe zone
            padding: [Left, Top, Right, Bottom] additional padding values
            
        Returns:
            Dict containing success status and safe zone properties
            
        Examples:
            # Add a basic safe zone
            add_safe_zone_to_widget(
                widget_name="MyWidget", 
                safe_zone_name="SafeContent"
            )
            
            # Add a customized safe zone with padding
            add_safe_zone_to_widget(
                widget_name="GameHUD",
                safe_zone_name="HUDSafeArea",
                position=[0.0, 0.0],
                size=[1920.0, 1080.0],
                padding=[20.0, 20.0, 20.0, 20.0]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": safe_zone_name,
                "position": position,
                "size": size,
                "padding": padding
            }
            
            logger.info(f"Adding Safe Zone to widget with params: {params}")
            response = unreal.send_command("add_safe_zone_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Safe Zone response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Safe Zone to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_invalidation_box_to_widget(
        ctx: Context,
        widget_name: str,
        invalidation_box_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [300.0, 300.0],
        cache_in_invalidation: bool = True
    ) -> Dict[str, Any]:
        """
        Add an Invalidation Box widget to a UMG Widget Blueprint.
        
        The Invalidation Box allows you to cache parts of your UI for performance optimization.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            invalidation_box_name: Name to give the new Invalidation Box widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the invalidation box
            cache_in_invalidation: Whether to enable caching
            
        Returns:
            Dict containing success status and invalidation box properties
            
        Examples:
            # Add a basic invalidation box
            add_invalidation_box_to_widget(
                widget_name="MyWidget", 
                invalidation_box_name="CachedContent"
            )
            
            # Add a customized invalidation box
            add_invalidation_box_to_widget(
                widget_name="ComplexUI",
                invalidation_box_name="StaticElementsCache",
                position=[100.0, 100.0],
                size=[500.0, 400.0],
                cache_in_invalidation=True
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": invalidation_box_name,
                "position": position,
                "size": size,
                "cache_in_invalidation": cache_in_invalidation
            }
            
            logger.info(f"Adding Invalidation Box to widget with params: {params}")
            response = unreal.send_command("add_invalidation_box_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Invalidation Box response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Invalidation Box to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_input_key_selector_to_widget(
        ctx: Context,
        widget_name: str,
        key_selector_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [200.0, 50.0],
        text_color: List[float] = [1.0, 1.0, 1.0, 1.0],
        selected_key: str = ""
    ) -> Dict[str, Any]:
        """
        Add an Input Key Selector widget to a UMG Widget Blueprint.
        
        A widget for selecting a single key or a single key with a modifier.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            key_selector_name: Name to give the new Input Key Selector widget
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the input key selector
            text_color: [R, G, B, A] color values for the text (0.0 to 1.0)
            selected_key: Initially selected key name (e.g., "W", "Gamepad_FaceButton_Bottom")
            
        Returns:
            Dict containing success status and input key selector properties
            
        Examples:
            # Add a basic input key selector
            add_input_key_selector_to_widget(
                widget_name="MyWidget", 
                key_selector_name="JumpKeySelector"
            )
            
            # Add a customized input key selector with a default key
            add_input_key_selector_to_widget(
                widget_name="ControlsMenu",
                key_selector_name="FireKeySelector",
                position=[200.0, 150.0],
                size=[250.0, 60.0],
                text_color=[0.9, 0.9, 0.9, 1.0],
                selected_key="LeftMouseButton"
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": key_selector_name,
                "position": position,
                "size": size,
                "text_color": text_color,
                "selected_key": selected_key
            }
            
            logger.info(f"Adding Input Key Selector to widget with params: {params}")
            response = unreal.send_command("add_input_key_selector_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Input Key Selector response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Input Key Selector to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_multi_line_editable_text_to_widget(
        ctx: Context,
        widget_name: str,
        text_box_name: str,
        hint_text: str = "",
        text: str = "",
        position: List[float] = [0.0, 0.0],
        size: List[float] = [300.0, 150.0],
        allows_multiline: bool = True
    ) -> Dict[str, Any]:
        """
        Add a Multi-Line Editable Text widget to a UMG Widget Blueprint.
        
        Editable text box widget for custom multi-line text entry.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            text_box_name: Name to give the new Multi-Line Editable Text widget
            hint_text: Hint text displayed when text box is empty
            text: Initial text content
            position: [X, Y] position in the canvas panel
            size: [Width, Height] of the text box
            allows_multiline: Whether to allow multiple lines of text
            
        Returns:
            Dict containing success status and multi-line editable text properties
            
        Examples:
            # Add a basic multi-line text box
            add_multi_line_editable_text_to_widget(
                widget_name="MyWidget", 
                text_box_name="CommentsBox",
                hint_text="Enter your comments here..."
            )
            
            # Add a customized multi-line text box with initial content
            add_multi_line_editable_text_to_widget(
                widget_name="FeedbackForm",
                text_box_name="FeedbackInput",
                hint_text="Please provide your feedback",
                text="I really enjoyed the game, especially the...",
                position=[100.0, 200.0],
                size=[400.0, 200.0]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": text_box_name,
                "hint_text": hint_text,
                "text": text,
                "position": position,
                "size": size,
                "allows_multiline": allows_multiline
            }
            
            logger.info(f"Adding Multi-Line Editable Text to widget with params: {params}")
            response = unreal.send_command("add_multi_line_editable_text_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Multi-Line Editable Text response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Multi-Line Editable Text to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_size_box_to_widget(
        ctx: Context,
        widget_name: str,
        size_box_name: str,
        position: List[float] = [0.0, 0.0],
        fixed_width: float = 200.0,
        fixed_height: float = 200.0,
        min_width: float = 0.0,
        min_height: float = 0.0,
        max_width: float = 0.0,
        max_height: float = 0.0
    ) -> Dict[str, Any]:
        """
        Add a Size Box widget to a UMG Widget Blueprint.
        
        A container that limits the size of its child widget.
        
        Args:
            widget_name: Name of the target Widget Blueprint
            size_box_name: Name to give the new Size Box widget
            position: [X, Y] position in the canvas panel
            fixed_width: Fixed width (0 means not fixed)
            fixed_height: Fixed height (0 means not fixed)
            min_width: Minimum width (0 means no minimum)
            min_height: Minimum height (0 means no minimum)
            max_width: Maximum width (0 means no maximum)
            max_height: Maximum height (0 means no maximum)
            
        Returns:
            Dict containing success status and size box properties
            
        Examples:
            # Add a basic size box with fixed dimensions
            add_size_box_to_widget(
                widget_name="MyWidget", 
                size_box_name="ContentSizer"
            )
            
            # Add a size box with min/max constraints
            add_size_box_to_widget(
                widget_name="ResponsiveUI",
                size_box_name="FlexibleContainer",
                position=[100.0, 100.0],
                fixed_width=0.0,
                fixed_height=0.0,
                min_width=200.0,
                min_height=150.0,
                max_width=500.0,
                max_height=350.0
            )
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "blueprint_name": widget_name,
                "widget_name": size_box_name,
                "position": position,
                "fixed_width": fixed_width,
                "fixed_height": fixed_height,
                "min_width": min_width,
                "min_height": min_height,
                "max_width": max_width,
                "max_height": max_height
            }
            
            logger.info(f"Adding Size Box to widget with params: {params}")
            response = unreal.send_command("add_size_box_to_widget", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Add Size Box response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding Size Box to widget: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    logger.info("UMG tools registered successfully")