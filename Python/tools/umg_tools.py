"""
UMG Tools for Unreal MCP.

This module provides tools for creating and manipulating UMG Widget Blueprints in Unreal Engine.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import FastMCP, Context
from utils.widgets.widget_components import (
    create_widget_blueprint as create_widget_blueprint_impl,
    add_text_block as add_text_block_impl,
    add_button as add_button_impl,
    add_image as add_image_impl,
    add_check_box as add_check_box_impl,
    bind_event as bind_event_impl,
    add_to_viewport as add_to_viewport_impl,
    set_text_binding as set_text_binding_impl,
    add_slider as add_slider_impl,
    add_progress_bar as add_progress_bar_impl,
    add_border as add_border_impl,
    add_scroll_box as add_scroll_box_impl,
    add_spacer as add_spacer_impl,
    add_widget_switcher as add_widget_switcher_impl,
    add_throbber as add_throbber_impl,
    add_expandable_area as add_expandable_area_impl,
    add_rich_text_block as add_rich_text_block_impl,
    add_multi_line_editable_text as add_multi_line_editable_text_impl,
    add_widget_as_child as add_widget_as_child_impl,
    create_widget_component_with_child as create_widget_component_with_child_impl,
    check_component_exists as check_component_exists_impl,
    set_widget_component_placement as set_widget_component_placement_impl,
    get_widget_container_dimensions as get_widget_container_dimensions_impl
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
        return create_widget_blueprint_impl(ctx, widget_name, parent_class, path)

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
    ) -> Dict[str, object]:
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
        return add_text_block_impl(ctx, widget_name, text_block_name, text, position, size, font_size, color)

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
    ) -> Dict[str, object]:
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
        return add_button_impl(ctx, widget_name, button_name, text, position, size, font_size, color, background_color)

    @mcp.tool()
    def add_image_to_widget(
        ctx: Context,
        widget_name: str, 
        image_name: str,
        brush_asset_path: str = "",
        position: List[float] = [0.0, 0.0],
        size: List[float] = [100.0, 100.0]
    ) -> Dict[str, object]:
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
        return add_image_impl(ctx, widget_name, image_name, brush_asset_path, position, size)

    @mcp.tool()
    def add_check_box_to_widget(
        ctx: Context,
        widget_name: str, 
        checkbox_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [40.0, 40.0],
        is_checked: bool = False
    ) -> Dict[str, object]:
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
        return add_check_box_impl(ctx, widget_name, checkbox_name, position, size, is_checked)

    @mcp.tool()
    def bind_widget_event(
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
        return bind_event_impl(ctx, widget_name, widget_component_name, event_name, function_name)

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
        return add_to_viewport_impl(ctx, widget_name, z_order)

    @mcp.tool()
    def set_text_block_binding(
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
        return set_text_binding_impl(ctx, widget_name, text_block_name, binding_property, binding_type)

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
    ) -> Dict[str, object]:
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
        return add_slider_impl(ctx, widget_name, slider_name, min_value, max_value, value, position, size, orientation, bar_color, handle_color)

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
    ) -> Dict[str, object]:
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
        return add_progress_bar_impl(ctx, widget_name, progress_bar_name, percent, position, size, fill_color, background_color)

    @mcp.tool()
    def add_border_to_widget(
        ctx: Context,
        widget_name: str,
        border_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [200.0, 200.0],
        brush_color: List[float] = [0.1, 0.1, 0.1, 1.0],
        brush_thickness: float = 4.0
    ) -> Dict[str, object]:
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
        return add_border_impl(ctx, widget_name, border_name, position, size, brush_color, brush_thickness)

    @mcp.tool()
    def add_scroll_box_to_widget(
        ctx: Context,
        widget_name: str,
        scroll_box_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [300.0, 200.0],
        orientation: str = "Vertical",
        scroll_bar_visibility: str = "Visible"
    ) -> Dict[str, object]:
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
        return add_scroll_box_impl(ctx, widget_name, scroll_box_name, position, size, orientation, scroll_bar_visibility)

    @mcp.tool()
    def add_spacer_to_widget(
        ctx: Context,
        widget_name: str,
        spacer_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [100.0, 100.0]
    ) -> Dict[str, object]:
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
        return add_spacer_impl(ctx, widget_name, spacer_name, position, size)

    @mcp.tool()
    def add_widget_switcher_to_widget(
        ctx: Context,
        widget_name: str,
        switcher_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [400.0, 300.0],
        active_widget_index: int = 0
    ) -> Dict[str, object]:
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
        return add_widget_switcher_impl(ctx, widget_name, switcher_name, position, size, active_widget_index)

    @mcp.tool()
    def add_throbber_to_widget(
        ctx: Context,
        widget_name: str,
        throbber_name: str,
        position: List[float] = [0.0, 0.0],
        size: List[float] = [100.0, 20.0],
        num_pieces: int = 3,
        animate: bool = True
    ) -> Dict[str, object]:
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
        return add_throbber_impl(ctx, widget_name, throbber_name, position, size, num_pieces, animate)

    @mcp.tool()
    def add_expandable_area_to_widget(
        ctx: Context,
        widget_name: str,
        expandable_area_name: str,
        header_text: str = "Header",
        position: List[float] = [0.0, 0.0],
        size: List[float] = [300.0, 100.0],
        is_expanded: bool = False
    ) -> Dict[str, object]:
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
        return add_expandable_area_impl(ctx, widget_name, expandable_area_name, header_text, position, size, is_expanded)

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
    ) -> Dict[str, object]:
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
        return add_rich_text_block_impl(ctx, widget_name, rich_text_name, text, position, size, font_size, default_color, auto_wrap_text)

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
    ) -> Dict[str, object]:
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
        return add_multi_line_editable_text_impl(ctx, widget_name, text_box_name, hint_text, text, position, size, allows_multiline)

    @mcp.tool()
    def add_widget_as_child(
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
            add_widget_as_child(
                widget_name="MyWidget",
                parent_component_name="ContentBorder",
                child_component_name="HeaderText"
            )
            
            # Add an existing button to a new vertical box (creates if missing)
            add_widget_as_child(
                widget_name="GameMenu",
                parent_component_name="ButtonsContainer",
                child_component_name="StartButton",
                create_parent_if_missing=True,
                parent_component_type="VerticalBox",
                parent_position=[100.0, 100.0],
                parent_size=[300.0, 400.0]
            )
        """
        return add_widget_as_child_impl(
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
    def create_widget_component_with_child(
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
            create_widget_component_with_child(
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
            create_widget_component_with_child(
                widget_name="InventoryScreen",
                parent_component_name="ItemsScrollBox",
                child_component_name="ItemsContainer",
                parent_component_type="ScrollBox",
                child_component_type="VerticalBox",
                parent_position=[100.0, 200.0],
                parent_size=[300.0, 400.0]
            )
        """
        if child_attributes is None:
            child_attributes = {}
            
        return create_widget_component_with_child_impl(
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
    def check_component_exists(
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
            check_component_exists(
                widget_name="MyWidget",
                component_name="HeaderText"
            )
        """
        return check_component_exists_impl(ctx, widget_name, component_name)

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
        return set_widget_component_placement_impl(ctx, widget_name, component_name, position, size, alignment)

    @mcp.tool()
    def get_widget_container_dimensions(
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
            dimensions = get_widget_container_dimensions(
                widget_name="MainMenu"
            )
            
            # Get dimensions of a specific container
            dimensions = get_widget_container_dimensions(
                widget_name="InventoryScreen",
                container_name="ItemsContainer"
            )
            
            # Use the dimensions to place a widget in the top-right corner with a 10px margin
            dimensions = get_widget_container_dimensions(widget_name="HUD")
            set_widget_component_placement(
                widget_name="HUD",
                component_name="CloseButton",
                position=[dimensions["width"] - 10, 10],
                alignment=[1.0, 0.0]
            )
        """
        return get_widget_container_dimensions_impl(ctx, widget_name, container_name)

    logger.info("UMG tools registered successfully")