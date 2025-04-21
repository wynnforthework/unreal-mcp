#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handles UMG (Widget Blueprint) related MCP commands
 * Responsible for creating and modifying UMG Widget Blueprints,
 * adding widget components, and managing widget instances in the viewport.
 */
class UNREALMCP_API FUnrealMCPUMGCommands
{
public:
    FUnrealMCPUMGCommands();

    /**
     * Handle UMG-related commands
     * @param CommandType - The type of command to handle
     * @param Params - JSON parameters for the command
     * @return JSON response with results or error
     */
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    /**
     * Create a new UMG Widget Blueprint
     * @param Params - Must include "name" for the blueprint name
     * @return JSON response with the created blueprint details
     */
    TSharedPtr<FJsonObject> HandleCreateUMGWidgetBlueprint(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Text Block widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Text Block
     *                "text" - Initial text content (optional)
     *                "position" - [X, Y] position in the canvas (optional)
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddTextBlockToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a widget instance to the game viewport
     * @param Params - Must include:
     *                "blueprint_name" - Name of the Widget Blueprint to instantiate
     *                "z_order" - Z-order for widget display (optional)
     * @return JSON response with the widget instance details
     */
    TSharedPtr<FJsonObject> HandleAddWidgetToViewport(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Button widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Button
     *                "text" - Button text
     *                "position" - [X, Y] position in the canvas
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddButtonToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Bind an event to a widget (e.g. button click)
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name of the widget to bind
     *                "event_name" - Name of the event to bind
     * @return JSON response with the binding details
     */
    TSharedPtr<FJsonObject> HandleBindWidgetEvent(const TSharedPtr<FJsonObject>& Params);

    /**
     * Set up text block binding for dynamic updates
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name of the widget to bind
     *                "binding_name" - Name of the binding to set up
     * @return JSON response with the binding details
     */
    TSharedPtr<FJsonObject> HandleSetTextBlockBinding(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Widget Switcher to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Widget Switcher
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddWidgetSwitcherToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Throbber widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Throbber widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddThrobberToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add an Expandable Area widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Expandable Area widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddExpandableAreaToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Menu Anchor widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Menu Anchor widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddMenuAnchorToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Rich Text Block widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Rich Text Block widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddRichTextBlockToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Safe Zone widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Safe Zone widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddSafeZoneToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add an Invalidation Box widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Invalidation Box widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddInvalidationBoxToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add an Input Key Selector widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Input Key Selector widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddInputKeySelectorToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Multi-Line Editable Text widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Multi-Line Editable Text widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddMultiLineEditableTextToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Size Box widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Size Box widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddSizeBoxToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add an Image widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Image widget
     *                "brush_asset_path" - Path to the image asset to use (optional)
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddImageToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a CheckBox widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new CheckBox widget
     *                "is_checked" - Initial checked state (optional)
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddCheckBoxToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Slider widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Slider widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddSliderToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Progress Bar widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Progress Bar widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddProgressBarToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Border widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Border widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddBorderToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a ScrollBox widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new ScrollBox widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddScrollBoxToWidget(const TSharedPtr<FJsonObject>& Params);

    /**
     * Add a Spacer widget to a UMG Widget Blueprint
     * @param Params - Must include:
     *                "blueprint_name" - Name of the target Widget Blueprint
     *                "widget_name" - Name for the new Spacer widget
     * @return JSON response with the added widget details
     */
    TSharedPtr<FJsonObject> HandleAddSpacerToWidget(const TSharedPtr<FJsonObject>& Params);
};