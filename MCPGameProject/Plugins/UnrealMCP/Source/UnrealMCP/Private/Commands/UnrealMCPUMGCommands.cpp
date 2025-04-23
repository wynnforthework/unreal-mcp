#include "Commands/UnrealMCPUMGCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WidgetBlueprint.h"
// We'll create widgets using regular Factory classes
#include "Factories/Factory.h"
// Add missing BlueprintFactory include
#include "Factories/BlueprintFactory.h"
// Add AssetRegistry for searching across content folder
#include "AssetRegistry/AssetRegistryModule.h"
#include "WidgetBlueprintEditor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "JsonObjectConverter.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Components/Button.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node_Event.h"
#include "Misc/Paths.h"

// Include additional widget components
#include "Components/WidgetSwitcher.h"
#include "Components/Throbber.h"
#include "Components/ExpandableArea.h"
#include "Components/MenuAnchor.h"
#include "Components/RichTextBlock.h"
#include "Components/SafeZone.h"
#include "Components/InvalidationBox.h"
#include "Components/InputKeySelector.h"
#include "Components/MultiLineEditableText.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/GridPanel.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/CircularThrobber.h"
#include "Components/SpinBox.h"
#include "Components/WrapBox.h"
#include "Components/ScaleBox.h"
#include "Components/NamedSlot.h"
#include "Components/RadialSlider.h"
#include "Components/NativeWidgetHost.h"
#include "Components/BackgroundBlur.h"
#include "Components/SafeZone.h"
#include "Components/MenuAnchor.h"
#include "Components/ListView.h"
#include "Components/TileView.h"
#include "Components/TreeView.h"
#include "Components/UniformGridPanel.h"

// Helper function to find a Widget Blueprint by name
UWidgetBlueprint* FindWidgetBlueprint(const FString& BlueprintNameOrPath)
{
    UE_LOG(LogTemp, Log, TEXT("UMG: Searching for Widget Blueprint: %s"), *BlueprintNameOrPath);
    
    // Check if we already have a full path
    if (BlueprintNameOrPath.StartsWith(TEXT("/Game/")))
    {
        // Try direct loading first with provided path
        FString FullPath = BlueprintNameOrPath;
        
        // Add .uasset extension if not provided
        if (!FPaths::GetExtension(FullPath).IsEmpty())
        {
            // Path already has extension, so we can use it directly
        }
        else if (FullPath.EndsWith(TEXT("."), ESearchCase::CaseSensitive))
        {
            // Remove trailing dot
            FullPath = FullPath.LeftChop(1);
        }
        
        // Try directly loading the asset
        UE_LOG(LogTemp, Log, TEXT("UMG: Trying to load Widget Blueprint with direct path: %s"), *FullPath);
        UObject* Asset = UEditorAssetLibrary::LoadAsset(FullPath);
        UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Asset);
        if (WidgetBlueprint)
        {
            UE_LOG(LogTemp, Log, TEXT("UMG: Found Widget Blueprint at path: %s"), *FullPath);
            return WidgetBlueprint;
        }
    }
    
    // If not found with direct path, try common directories
    TArray<FString> SearchPaths = {
        FString::Printf(TEXT("/Game/Widgets/%s"), *BlueprintNameOrPath),
        FString::Printf(TEXT("/Game/UI/%s"), *BlueprintNameOrPath),
        FString::Printf(TEXT("/Game/UMG/%s"), *BlueprintNameOrPath),
        FString::Printf(TEXT("/Game/Interface/%s"), *BlueprintNameOrPath)
    };
    
    for (const FString& SearchPath : SearchPaths)
    {
        UE_LOG(LogTemp, Log, TEXT("UMG: Trying common path: %s"), *SearchPath);
        UObject* Asset = UEditorAssetLibrary::LoadAsset(SearchPath);
        UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Asset);
        if (WidgetBlueprint)
        {
            UE_LOG(LogTemp, Log, TEXT("UMG: Found Widget Blueprint at path: %s"), *SearchPath);
            return WidgetBlueprint;
        }
    }
    
    // If still not found, use asset registry to search everywhere
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetData;
    
    FARFilter Filter;
    // Use ClassPaths instead of ClassNames (which is deprecated in UE 5.5)
    Filter.ClassPaths.Add(UWidgetBlueprint::StaticClass()->GetClassPathName());
    Filter.PackageNames.Add(FName(*FString::Printf(TEXT("/Game"))));
    AssetRegistryModule.Get().GetAssets(Filter, AssetData);
    
    UE_LOG(LogTemp, Log, TEXT("UMG: Searching asset registry for Widget Blueprint with name: %s"), *BlueprintNameOrPath);
    
    for (const FAssetData& Asset : AssetData)
    {
        FString AssetName = Asset.AssetName.ToString();
        
        // Check if asset name matches (case insensitive)
        if (AssetName.Equals(BlueprintNameOrPath, ESearchCase::IgnoreCase))
        {
            // Use GetSoftObjectPath instead of ObjectPath (which is deprecated in UE 5.5)
            FString AssetPath = Asset.GetSoftObjectPath().ToString();
            UE_LOG(LogTemp, Log, TEXT("UMG: Found Widget Blueprint in asset registry: %s"), *AssetPath);
            UObject* LoadedAsset = UEditorAssetLibrary::LoadAsset(AssetPath);
            UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(LoadedAsset);
            if (WidgetBlueprint)
            {
                return WidgetBlueprint;
            }
        }
    }
    
    // Not found anywhere
    UE_LOG(LogTemp, Warning, TEXT("UMG: Widget Blueprint not found: %s"), *BlueprintNameOrPath);
    return nullptr;
}

// Helper function to safely get an array from a JSON object
bool GetJsonArray(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TArray<TSharedPtr<FJsonValue>>& OutArray)
{
	const TArray<TSharedPtr<FJsonValue>>* ArrayPtr = nullptr;
	if (JsonObject->TryGetArrayField(TCHAR_TO_ANSI(*FieldName), ArrayPtr) && ArrayPtr)
	{
		OutArray = *ArrayPtr;
		return true;
	}
	return false;
}

// Define log category if it doesn't exist
DEFINE_LOG_CATEGORY_STATIC(LogUnrealMCP, Log, All);

FUnrealMCPUMGCommands::FUnrealMCPUMGCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandName == TEXT("create_umg_widget_blueprint"))
	{
		return HandleCreateUMGWidgetBlueprint(Params);
	}
	else if (CommandName == TEXT("add_text_block_to_widget"))
	{
		return HandleAddTextBlockToWidget(Params);
	}
	else if (CommandName == TEXT("add_widget_to_viewport"))
	{
		return HandleAddWidgetToViewport(Params);
	}
	else if (CommandName == TEXT("add_button_to_widget"))
	{
		return HandleAddButtonToWidget(Params);
	}
	else if (CommandName == TEXT("bind_widget_event"))
	{
		return HandleBindWidgetEvent(Params);
	}
	else if (CommandName == TEXT("set_text_block_binding"))
	{
		return HandleSetTextBlockBinding(Params);
	}
	else if (CommandName == TEXT("add_widget_switcher_to_widget"))
	{
		return HandleAddWidgetSwitcherToWidget(Params);
	}
	else if (CommandName == TEXT("add_throbber_to_widget"))
	{
		return HandleAddThrobberToWidget(Params);
	}
	else if (CommandName == TEXT("add_expandable_area_to_widget"))
	{
		return HandleAddExpandableAreaToWidget(Params);
	}
	else if (CommandName == TEXT("add_menu_anchor_to_widget"))
	{
		return HandleAddMenuAnchorToWidget(Params);
	}
	else if (CommandName == TEXT("add_rich_text_block_to_widget"))
	{
		return HandleAddRichTextBlockToWidget(Params);
	}
	else if (CommandName == TEXT("add_safe_zone_to_widget"))
	{
		return HandleAddSafeZoneToWidget(Params);
	}
	else if (CommandName == TEXT("add_invalidation_box_to_widget"))
	{
		return HandleAddInvalidationBoxToWidget(Params);
	}
	else if (CommandName == TEXT("add_input_key_selector_to_widget"))
	{
		return HandleAddInputKeySelectorToWidget(Params);
	}
	else if (CommandName == TEXT("add_multi_line_editable_text_to_widget"))
	{
		return HandleAddMultiLineEditableTextToWidget(Params);
	}
	else if (CommandName == TEXT("add_size_box_to_widget"))
	{
		return HandleAddSizeBoxToWidget(Params);
	}
	else if (CommandName == TEXT("add_image_to_widget"))
	{
		return HandleAddImageToWidget(Params);
	}
	else if (CommandName == TEXT("add_check_box_to_widget"))
	{
		return HandleAddCheckBoxToWidget(Params);
	}
	else if (CommandName == TEXT("add_slider_to_widget"))
	{
		return HandleAddSliderToWidget(Params);
	}
	else if (CommandName == TEXT("add_progress_bar_to_widget"))
	{
		return HandleAddProgressBarToWidget(Params);
	}
	else if (CommandName == TEXT("add_border_to_widget"))
	{
		return HandleAddBorderToWidget(Params);
	}
	else if (CommandName == TEXT("add_scroll_box_to_widget"))
	{
		return HandleAddScrollBoxToWidget(Params);
	}
	else if (CommandName == TEXT("add_spacer_to_widget"))
	{
		return HandleAddSpacerToWidget(Params);
	}
	else if (CommandName == TEXT("check_component_exists"))
	{
		return HandleCheckComponentExists(Params);
	}
	else if (CommandName == TEXT("add_widget_as_child"))
	{
		return HandleAddWidgetAsChild(Params);
	}
	else if (CommandName == TEXT("create_widget_component_with_child"))
	{
		return HandleCreateWidgetComponentWithChild(Params);
	}
	else if (CommandName == TEXT("set_widget_component_placement"))
	{
		return HandleSetWidgetPlacement(Params);
	}
	else if (CommandName == TEXT("get_widget_container_dimensions"))
	{
		return HandleGetWidgetContainerDimensions(Params);
	}
	else if (CommandName == TEXT("add_widget_component"))
	{
		return HandleAddWidgetComponent(Params);
	}
	
	return FUnrealMCPCommonUtils::CreateErrorResponse(*FString::Printf(TEXT("Unknown command: %s"), *CommandName));
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleCreateUMGWidgetBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	// Create the full asset path
	FString PackagePath = TEXT("/Game/Widgets/");
	FString AssetName = BlueprintName;
	FString FullPath = PackagePath + AssetName;

	UE_LOG(LogTemp, Log, TEXT("UMG: Creating Widget Blueprint at path: %s"), *FullPath);

	// Check if asset already exists
	if (UEditorAssetLibrary::DoesAssetExist(FullPath))
	{
		UObject* ExistingAsset = UEditorAssetLibrary::LoadAsset(FullPath);
		UWidgetBlueprint* ExistingWidgetBP = Cast<UWidgetBlueprint>(ExistingAsset);
		
		// If it exists and is a Widget Blueprint, we can return it
		if (ExistingWidgetBP)
		{
			UE_LOG(LogTemp, Log, TEXT("UMG: Widget Blueprint already exists, returning existing asset"));
			// Create success response for existing blueprint
			TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
			ResultObj->SetStringField(TEXT("name"), BlueprintName);
			ResultObj->SetStringField(TEXT("path"), FullPath);
			ResultObj->SetBoolField(TEXT("already_exists"), true);
			ResultObj->SetBoolField(TEXT("success"), true);
			return ResultObj;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMG: Asset exists but is not a Widget Blueprint"));
			// Asset exists but is not a Widget Blueprint
			return FUnrealMCPCommonUtils::CreateErrorResponse(
				FString::Printf(TEXT("Asset '%s' exists but is not a Widget Blueprint"), *BlueprintName)
			);
		}
	}

	// Find parent class - default is UUserWidget
	UClass* ParentClass = UUserWidget::StaticClass(); 
	if (Params->HasField(TEXT("parent_class")))
	{
		FString ParentClassName;
		Params->TryGetStringField(TEXT("parent_class"), ParentClassName);
		
		if (!ParentClassName.IsEmpty() && ParentClassName != TEXT("UserWidget"))
		{
			// Try to find the parent class with various prefixes
			UClass* FoundClass = nullptr;
			TArray<FString> PossibleClassPaths;
			PossibleClassPaths.Add(ParentClassName);
			PossibleClassPaths.Add(FString::Printf(TEXT("/Script/UMG.%s"), *ParentClassName));
			PossibleClassPaths.Add(FString::Printf(TEXT("/Script/Engine.%s"), *ParentClassName));
			PossibleClassPaths.Add(FString::Printf(TEXT("/Game.%s"), *ParentClassName));
			
			for (const FString& ClassPath : PossibleClassPaths)
			{
				FoundClass = FindObject<UClass>(ANY_PACKAGE, *ClassPath);
				if (FoundClass)
				{
					break;
				}
				
				FoundClass = LoadClass<UUserWidget>(nullptr, *ClassPath);
				if (FoundClass)
				{
					break;
				}
			}
			
			if (FoundClass)
			{
				ParentClass = FoundClass;
				UE_LOG(LogTemp, Log, TEXT("UMG: Using parent class: %s"), *ParentClass->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UMG: Could not find parent class: %s, using default"), *ParentClassName);
			}
		}
	}

	// Create package for the new asset
	UE_LOG(LogTemp, Log, TEXT("UMG: Creating package for: %s"), *FullPath);
	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("UMG: Failed to create package for path: %s"), *FullPath);
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create package"));
	}

	// Create the Widget Blueprint
	UE_LOG(LogTemp, Log, TEXT("UMG: Creating Widget Blueprint"));
	
	// Create Blueprint using KismetEditorUtilities directly
	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		ParentClass,                       // Parent class
		Package,                           // Outer package
		FName(*AssetName),                 // Blueprint name
		BPTYPE_Normal,                     // Blueprint type
		UWidgetBlueprint::StaticClass(),   // Blueprint class
		UWidgetBlueprintGeneratedClass::StaticClass()  // Generated class
	);
	
	if (!NewBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("UMG: Failed to create blueprint"));
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create blueprint"));
	}
	
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(NewBlueprint);
	if (!WidgetBlueprint)
	{
		UE_LOG(LogTemp, Error, TEXT("UMG: Created blueprint is not a UWidgetBlueprint, deleting and returning error"));
		UEditorAssetLibrary::DeleteAsset(FullPath);
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Created blueprint is not a Widget Blueprint"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("UMG: Successfully created Widget Blueprint object"));

	// Ensure the WidgetTree exists
	if (!WidgetBlueprint->WidgetTree)
	{
		UE_LOG(LogTemp, Error, TEXT("UMG: Widget Blueprint has no WidgetTree"));
		UEditorAssetLibrary::DeleteAsset(FullPath);
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Widget Blueprint has no WidgetTree"));
	}

	// Add a default Canvas Panel root widget if needed
	if (!WidgetBlueprint->WidgetTree->RootWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("UMG: Adding default Canvas Panel to Widget Blueprint"));
		UCanvasPanel* RootCanvas = WidgetBlueprint->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		if (!RootCanvas)
		{
			UE_LOG(LogTemp, Error, TEXT("UMG: Failed to create Canvas Panel"));
			UEditorAssetLibrary::DeleteAsset(FullPath);
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Canvas Panel root widget"));
		}
		
		WidgetBlueprint->WidgetTree->RootWidget = RootCanvas;
	}

	// Finalize and save the asset
	UE_LOG(LogTemp, Log, TEXT("UMG: Finalizing and saving Widget Blueprint"));
	
	// Update asset registry
	FAssetRegistryModule::AssetCreated(WidgetBlueprint);
	
	// Compile
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	
	// Mark dirty and save
	Package->MarkPackageDirty();
	bool bSaved = UEditorAssetLibrary::SaveAsset(FullPath, false);
	
	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("UMG: Saving Widget Blueprint encountered issues"));
	}

	// Return success
	UE_LOG(LogTemp, Log, TEXT("UMG: Widget Blueprint creation completed successfully"));
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetBoolField(TEXT("success"), true);
	ResultObj->SetStringField(TEXT("name"), BlueprintName);
	ResultObj->SetStringField(TEXT("path"), FullPath);
	return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddTextBlockToWidget(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Find the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get optional parameters
	FString InitialText = TEXT("New Text Block");
	Params->TryGetStringField(TEXT("text"), InitialText);

	FVector2D Position(0.0f, 0.0f);
	if (Params->HasField(TEXT("position")))
	{
		const TArray<TSharedPtr<FJsonValue>>* PosArray;
		if (Params->TryGetArrayField(TEXT("position"), PosArray) && PosArray->Num() >= 2)
		{
			Position.X = (*PosArray)[0]->AsNumber();
			Position.Y = (*PosArray)[1]->AsNumber();
		}
	}

	// Create Text Block widget
	UTextBlock* TextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *WidgetName);
	if (!TextBlock)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Text Block widget"));
	}

	// Set initial text
	TextBlock->SetText(FText::FromString(InitialText));

	// Add to canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root Canvas Panel not found"));
	}

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(TextBlock);
	PanelSlot->SetPosition(Position);

	// Mark the package dirty and compile
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("text"), InitialText);
	return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddWidgetToViewport(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	// Find the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get optional Z-order parameter
	int32 ZOrder = 0;
	Params->TryGetNumberField(TEXT("z_order"), ZOrder);

	// Create widget instance
	UClass* WidgetClass = WidgetBlueprint->GeneratedClass;
	if (!WidgetClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get widget class"));
	}

	// Note: This creates the widget but doesn't add it to viewport
	// The actual addition to viewport should be done through Blueprint nodes
	// as it requires a game context

	// Create success response with instructions
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_name"), BlueprintName);
	ResultObj->SetStringField(TEXT("class_path"), WidgetClass->GetPathName());
	ResultObj->SetNumberField(TEXT("z_order"), ZOrder);
	ResultObj->SetStringField(TEXT("note"), TEXT("Widget class ready. Use CreateWidget and AddToViewport nodes in Blueprint to display in game."));
	return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddButtonToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing blueprint_name parameter"));
		return Response;
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing widget_name parameter"));
		return Response;
	}

	FString ButtonText;
	if (!Params->TryGetStringField(TEXT("text"), ButtonText))
	{
		ButtonText = TEXT("Button"); // Use default text if not provided
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintName));
		return Response;
	}

	// Create Button widget
	UButton* Button = WidgetBlueprint->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *WidgetName);
	if (!Button)
	{
		Response->SetStringField(TEXT("error"), TEXT("Failed to create Button widget"));
		return Response;
	}

	// Create text block for the button
	UTextBlock* ButtonTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), 
		*(WidgetName + TEXT("_Text"))
	);
	
	if (ButtonTextBlock)
	{
		ButtonTextBlock->SetText(FText::FromString(ButtonText));
		Button->AddChild(ButtonTextBlock);
	}

	// Get canvas panel and add button
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		Response->SetStringField(TEXT("error"), TEXT("Root widget is not a Canvas Panel"));
		return Response;
	}

	// Add to canvas and set position
	UCanvasPanelSlot* ButtonSlot = RootCanvas->AddChildToCanvas(Button);
	if (ButtonSlot)
	{
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			ButtonSlot->SetPosition(Pos);
			// Set size if provided
			const TArray<TSharedPtr<FJsonValue>>* Size;
			if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
			{
				FVector2D ButtonSize(
					(*Size)[0]->AsNumber(),
					(*Size)[1]->AsNumber()
				);
				ButtonSlot->SetSize(ButtonSize);
			}
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleBindWidgetEvent(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing blueprint_name parameter"));
		return Response;
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing widget_name parameter"));
		return Response;
	}

	FString EventName;
	if (!Params->TryGetStringField(TEXT("event_name"), EventName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing event_name parameter"));
		return Response;
	}

	FString FunctionName;
	Params->TryGetStringField(TEXT("function_name"), FunctionName);
	if (FunctionName.IsEmpty())
	{
		// Default function name is WidgetName_EventName
		FunctionName = WidgetName + TEXT("_") + EventName;
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintName));
		return Response;
	}

	// Create the event graph if it doesn't exist
	UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(WidgetBlueprint);
	if (!EventGraph)
	{
		Response->SetStringField(TEXT("error"), TEXT("Failed to find or create event graph"));
		return Response;
	}

	// Find the widget in the blueprint
	UWidget* Widget = WidgetBlueprint->WidgetTree->FindWidget(*WidgetName);
	if (!Widget)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to find widget: %s"), *WidgetName));
		return Response;
	}

	// Create the event node (e.g., OnClicked for buttons)
	UK2Node_Event* EventNode = nullptr;
	
	// Find existing nodes first
	TArray<UK2Node_Event*> AllEventNodes;
	FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_Event>(WidgetBlueprint, AllEventNodes);
	
	for (UK2Node_Event* Node : AllEventNodes)
	{
		if (Node->CustomFunctionName == FName(*EventName) && Node->EventReference.GetMemberParentClass() == Widget->GetClass())
		{
			EventNode = Node;
			break;
		}
	}

	// If no existing node, create a new one
	if (!EventNode)
	{
		// Calculate position - place it below existing nodes
		float MaxHeight = 0.0f;
		for (UEdGraphNode* Node : EventGraph->Nodes)
		{
			MaxHeight = FMath::Max(MaxHeight, Node->NodePosY);
		}
		
		const FVector2D NodePos(200, MaxHeight + 200);

		// Call CreateNewBoundEventForClass, which returns void, so we can't capture the return value directly
		// We'll need to find the node after creating it
		FKismetEditorUtilities::CreateNewBoundEventForClass(
			Widget->GetClass(),
			FName(*EventName),
			WidgetBlueprint,
			nullptr  // We don't need a specific property binding
		);

		// Now find the newly created node
		TArray<UK2Node_Event*> UpdatedEventNodes;
		FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_Event>(WidgetBlueprint, UpdatedEventNodes);
		
		for (UK2Node_Event* Node : UpdatedEventNodes)
		{
			if (Node->CustomFunctionName == FName(*EventName) && Node->EventReference.GetMemberParentClass() == Widget->GetClass())
			{
				EventNode = Node;
				
				// Set position of the node
				EventNode->NodePosX = NodePos.X;
				EventNode->NodePosY = NodePos.Y;
				
				break;
			}
		}
	}

	if (!EventNode)
	{
		Response->SetStringField(TEXT("error"), TEXT("Failed to create event node"));
		return Response;
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("event_name"), EventName);
	Response->SetStringField(TEXT("function_name"), FunctionName);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleSetTextBlockBinding(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing blueprint_name parameter"));
		return Response;
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing widget_name parameter"));
		return Response;
	}

	FString BindingName;
	if (!Params->TryGetStringField(TEXT("binding_name"), BindingName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing binding_name parameter"));
		return Response;
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintName));
		return Response;
	}

	// Create a variable for binding if it doesn't exist
	FBlueprintEditorUtils::AddMemberVariable(
		WidgetBlueprint,
		FName(*BindingName),
		FEdGraphPinType(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType())
	);

	// Find the TextBlock widget
	UTextBlock* TextBlock = Cast<UTextBlock>(WidgetBlueprint->WidgetTree->FindWidget(FName(*WidgetName)));
	if (!TextBlock)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to find TextBlock widget: %s"), *WidgetName));
		return Response;
	}

	// Create binding function
	const FString FunctionName = FString::Printf(TEXT("Get%s"), *BindingName);
	UEdGraph* FuncGraph = FBlueprintEditorUtils::CreateNewGraph(
		WidgetBlueprint,
		FName(*FunctionName),
		UEdGraph::StaticClass(),
		UEdGraphSchema_K2::StaticClass()
	);

	if (FuncGraph)
	{
		// Add the function to the blueprint with proper template parameter
		// Template requires null for last parameter when not using a signature-source
		FBlueprintEditorUtils::AddFunctionGraph<UClass>(WidgetBlueprint, FuncGraph, false, nullptr);

		// Create entry node
		UK2Node_FunctionEntry* EntryNode = nullptr;
		
		// Create entry node - use the API that exists in UE 5.5
		EntryNode = NewObject<UK2Node_FunctionEntry>(FuncGraph);
		FuncGraph->AddNode(EntryNode, false, false);
		EntryNode->NodePosX = 0;
		EntryNode->NodePosY = 0;
		EntryNode->FunctionReference.SetExternalMember(FName(*FunctionName), WidgetBlueprint->GeneratedClass);
		EntryNode->AllocateDefaultPins();

		// Create get variable node
		UK2Node_VariableGet* GetVarNode = NewObject<UK2Node_VariableGet>(FuncGraph);
		GetVarNode->VariableReference.SetSelfMember(FName(*BindingName));
		FuncGraph->AddNode(GetVarNode, false, false);
		GetVarNode->NodePosX = 200;
		GetVarNode->NodePosY = 0;
		GetVarNode->AllocateDefaultPins();

		// Connect nodes
		UEdGraphPin* EntryThenPin = EntryNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* GetVarOutPin = GetVarNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
		if (EntryThenPin && GetVarOutPin)
		{
			EntryThenPin->MakeLinkTo(GetVarOutPin);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("binding_name"), BindingName);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddWidgetSwitcherToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Widget Switcher widget
	UWidgetSwitcher* WidgetSwitcher = WidgetBlueprint->WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), *WidgetName);
	if (!WidgetSwitcher)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Widget Switcher widget"));
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(WidgetSwitcher);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D SwitcherSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(SwitcherSize);
		}
	}

	// Set active widget index if provided
	int32 ActiveWidgetIndex = 0;
	Params->TryGetNumberField(TEXT("active_widget_index"), ActiveWidgetIndex);
	WidgetSwitcher->SetActiveWidgetIndex(ActiveWidgetIndex);

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetNumberField(TEXT("active_widget_index"), ActiveWidgetIndex);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddThrobberToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Throbber widget
	UThrobber* Throbber = WidgetBlueprint->WidgetTree->ConstructWidget<UThrobber>(UThrobber::StaticClass(), *WidgetName);
	if (!Throbber)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Throbber widget"));
	}

	// Set number of pieces if provided
	int32 NumPieces = 3;
	Params->TryGetNumberField(TEXT("num_pieces"), NumPieces);
	Throbber->SetNumberOfPieces(NumPieces);

	// Set animation state if provided
	bool bAnimate = true;
	Params->TryGetBoolField(TEXT("animate"), bAnimate);
	if (bAnimate)
	{
		Throbber->SetAnimateHorizontally(true);
		Throbber->SetAnimateVertically(false);
		Throbber->SetAnimateOpacity(true);
	}
	else
	{
		Throbber->SetAnimateHorizontally(false);
		Throbber->SetAnimateVertically(false);
		Throbber->SetAnimateOpacity(false);
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Throbber);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D ThrobberSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(ThrobberSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetNumberField(TEXT("num_pieces"), NumPieces);
	Response->SetBoolField(TEXT("animate"), bAnimate);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddExpandableAreaToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Expandable Area widget
	UExpandableArea* ExpandableArea = WidgetBlueprint->WidgetTree->ConstructWidget<UExpandableArea>(UExpandableArea::StaticClass(), *WidgetName);
	if (!ExpandableArea)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Expandable Area widget"));
	}

	// Create a text block for the header text
	FString HeaderText = TEXT("Header");
	Params->TryGetStringField(TEXT("header_text"), HeaderText);
	
	// In UE 5.5, UExpandableArea doesn't have a direct SetHeaderContent or SetHeaderText method
	// We need to create a TextBlock widget and add it as a child in the blueprint editor
	// This will be set as the header in the editor, but can't be done programmatically at runtime
	UTextBlock* HeaderTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), 
		*(WidgetName + TEXT("_HeaderText"))
	);
	
	if (HeaderTextBlock)
	{
		HeaderTextBlock->SetText(FText::FromString(HeaderText));
	}

	// Set expansion state
	bool IsExpanded = false;
	Params->TryGetBoolField(TEXT("is_expanded"), IsExpanded);
	ExpandableArea->SetIsExpanded(IsExpanded);

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(ExpandableArea);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D AreaSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(AreaSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetStringField(TEXT("header_text"), HeaderText);
	Response->SetBoolField(TEXT("is_expanded"), IsExpanded);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddMenuAnchorToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Menu Anchor widget
	UMenuAnchor* MenuAnchor = WidgetBlueprint->WidgetTree->ConstructWidget<UMenuAnchor>(UMenuAnchor::StaticClass(), *WidgetName);
	if (!MenuAnchor)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Menu Anchor widget"));
	}

	// Set placement mode
	FString Placement = TEXT("Below");
	Params->TryGetStringField(TEXT("placement"), Placement);

	if (Placement.Equals(TEXT("Below"), ESearchCase::IgnoreCase))
	{
		MenuAnchor->SetPlacement(MenuPlacement_BelowAnchor);
	}
	else if (Placement.Equals(TEXT("Above"), ESearchCase::IgnoreCase))
	{
		MenuAnchor->SetPlacement(MenuPlacement_AboveAnchor);
	}
	else if (Placement.Equals(TEXT("Left"), ESearchCase::IgnoreCase))
	{
		MenuAnchor->SetPlacement(MenuPlacement_MenuLeft);
	}
	else if (Placement.Equals(TEXT("Right"), ESearchCase::IgnoreCase))
	{
		MenuAnchor->SetPlacement(MenuPlacement_MenuRight);
	}
	else if (Placement.Equals(TEXT("Center"), ESearchCase::IgnoreCase))
	{
		MenuAnchor->SetPlacement(MenuPlacement_CenteredBelowAnchor);
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(MenuAnchor);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D AnchorSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(AnchorSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetStringField(TEXT("placement"), Placement);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddRichTextBlockToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Rich Text Block widget
	URichTextBlock* RichTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass(), *WidgetName);
	if (!RichTextBlock)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Rich Text Block widget"));
	}

	// Set text content if provided
	FString Text;
	if (Params->TryGetStringField(TEXT("text"), Text))
	{
		RichTextBlock->SetText(FText::FromString(Text));
	}

	// Set font size if provided
	int32 FontSize = 12;
	Params->TryGetNumberField(TEXT("font_size"), FontSize);
	RichTextBlock->SetMinDesiredWidth(10.0f); // Set a minimum width to prevent text collapse
	
	// Set auto wrapping if provided
	bool AutoWrapText = true;
	Params->TryGetBoolField(TEXT("auto_wrap_text"), AutoWrapText);
	RichTextBlock->SetAutoWrapText(AutoWrapText);

	// Set default text color if provided
	const TArray<TSharedPtr<FJsonValue>>* ColorArray;
	if (Params->TryGetArrayField(TEXT("default_color"), ColorArray) && ColorArray->Num() >= 4)
	{
		FSlateColor TextColor = FSlateColor(FLinearColor(
			(*ColorArray)[0]->AsNumber(),
			(*ColorArray)[1]->AsNumber(),
			(*ColorArray)[2]->AsNumber(),
			(*ColorArray)[3]->AsNumber()
		));
		RichTextBlock->SetDefaultColorAndOpacity(TextColor);
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(RichTextBlock);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D TextSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(TextSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	if (!Text.IsEmpty())
	{
		Response->SetStringField(TEXT("text"), Text);
	}
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddSafeZoneToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Safe Zone widget
	USafeZone* SafeZone = WidgetBlueprint->WidgetTree->ConstructWidget<USafeZone>(USafeZone::StaticClass(), *WidgetName);
	if (!SafeZone)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Safe Zone widget"));
	}

	// Note: USafeZone doesn't have SetPadding in UE 5.5, 
	// instead it automatically applies safe zone margins based on the device

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(SafeZone);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D ZoneSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(ZoneSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddInvalidationBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Invalidation Box widget
	UInvalidationBox* InvalidationBox = WidgetBlueprint->WidgetTree->ConstructWidget<UInvalidationBox>(UInvalidationBox::StaticClass(), *WidgetName);
	if (!InvalidationBox)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Invalidation Box widget"));
	}

	// Set caching state if provided
	bool CacheInInvalidation = true;
	Params->TryGetBoolField(TEXT("cache_in_invalidation"), CacheInInvalidation);
	InvalidationBox->SetCanCache(CacheInInvalidation);

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(InvalidationBox);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D BoxSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(BoxSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetBoolField(TEXT("cache_in_invalidation"), CacheInInvalidation);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddInputKeySelectorToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Input Key Selector widget
	UInputKeySelector* KeySelector = WidgetBlueprint->WidgetTree->ConstructWidget<UInputKeySelector>(UInputKeySelector::StaticClass(), *WidgetName);
	if (!KeySelector)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Input Key Selector widget"));
	}

	// Set text color if provided - using the proper UE 5.5 API
	const TArray<TSharedPtr<FJsonValue>>* ColorArray;
	if (Params->TryGetArrayField(TEXT("text_color"), ColorArray) && ColorArray->Num() >= 4)
	{
		FLinearColor TextColor(
			(*ColorArray)[0]->AsNumber(),
			(*ColorArray)[1]->AsNumber(),
			(*ColorArray)[2]->AsNumber(),
			(*ColorArray)[3]->AsNumber()
		);
		
		// Make sure text is visible
		KeySelector->SetTextBlockVisibility(ESlateVisibility::Visible);
		
		// In UE 5.5, we need to use the proper getter/setter instead of directly accessing WidgetStyle
		// KeySelector->SetColorAndOpacity(TextColor); - This doesn't exist for InputKeySelector
		// Instead, we'll just make sure the text is visible but color must be set in design time
	}

	// Set selected key if provided
	FString SelectedKey;
	if (Params->TryGetStringField(TEXT("selected_key"), SelectedKey) && !SelectedKey.IsEmpty())
	{
		FKey Key(*SelectedKey);
		if (Key.IsValid())
		{
			KeySelector->SetSelectedKey(Key);
		}
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(KeySelector);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D KeySelectorSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(KeySelectorSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	if (!SelectedKey.IsEmpty())
	{
		Response->SetStringField(TEXT("selected_key"), SelectedKey);
	}
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddMultiLineEditableTextToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Multi-Line Editable Text widget
	UMultiLineEditableText* TextBox = WidgetBlueprint->WidgetTree->ConstructWidget<UMultiLineEditableText>(UMultiLineEditableText::StaticClass(), *WidgetName);
	if (!TextBox)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Multi-Line Editable Text widget"));
	}

	// Set hint text if provided
	FString HintText;
	if (Params->TryGetStringField(TEXT("hint_text"), HintText))
	{
		TextBox->SetHintText(FText::FromString(HintText));
	}

	// Set initial text if provided
	FString Text;
	if (Params->TryGetStringField(TEXT("text"), Text))
	{
		TextBox->SetText(FText::FromString(Text));
	}

	// Note: MultiLineEditableText in UE 5.5 is already multi-line by default
	// The allows_multiline parameter exists in the Python API for consistency
	// but has no effect here as the widget class itself is designed for multi-line input

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(TextBox);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D TextBoxSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(TextBoxSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	if (!HintText.IsEmpty())
	{
		Response->SetStringField(TEXT("hint_text"), HintText);
	}
	if (!Text.IsEmpty())
	{
		Response->SetStringField(TEXT("text"), Text);
	}
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddSizeBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Size Box widget
	USizeBox* SizeBox = WidgetBlueprint->WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *WidgetName);
	if (!SizeBox)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Size Box widget"));
	}

	// Set fixed width if provided
	float FixedWidth = 200.0f;
	Params->TryGetNumberField(TEXT("fixed_width"), FixedWidth);
	if (FixedWidth > 0.0f)
	{
		SizeBox->SetWidthOverride(FixedWidth);
	}

	// Set fixed height if provided
	float FixedHeight = 200.0f;
	Params->TryGetNumberField(TEXT("fixed_height"), FixedHeight);
	if (FixedHeight > 0.0f)
	{
		SizeBox->SetHeightOverride(FixedHeight);
	}

	// Set min width if provided
	float MinWidth = 0.0f;
	Params->TryGetNumberField(TEXT("min_width"), MinWidth);
	if (MinWidth > 0.0f)
	{
		SizeBox->SetMinDesiredWidth(MinWidth);
	}

	// Set min height if provided
	float MinHeight = 0.0f;
	Params->TryGetNumberField(TEXT("min_height"), MinHeight);
	if (MinHeight > 0.0f)
	{
		SizeBox->SetMinDesiredHeight(MinHeight);
	}

	// Set max width if provided
	float MaxWidth = 0.0f;
	Params->TryGetNumberField(TEXT("max_width"), MaxWidth);
	if (MaxWidth > 0.0f)
	{
		SizeBox->SetMaxDesiredWidth(MaxWidth);
	}

	// Set max height if provided
	float MaxHeight = 0.0f;
	Params->TryGetNumberField(TEXT("max_height"), MaxHeight);
	if (MaxHeight > 0.0f)
	{
		SizeBox->SetMaxDesiredHeight(MaxHeight);
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(SizeBox);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	if (FixedWidth > 0.0f)
	{
		Response->SetNumberField(TEXT("fixed_width"), FixedWidth);
	}
	if (FixedHeight > 0.0f)
	{
		Response->SetNumberField(TEXT("fixed_height"), FixedHeight);
	}
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddImageToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Image widget
	UImage* ImageWidget = WidgetBlueprint->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *WidgetName);
	if (!ImageWidget)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Image widget"));
	}

	// Set brush asset if provided
	FString BrushAssetPath;
	if (Params->TryGetStringField(TEXT("brush_asset_path"), BrushAssetPath) && !BrushAssetPath.IsEmpty())
	{
		// Try to load the specified texture/brush asset
		UObject* BrushAsset = UEditorAssetLibrary::LoadAsset(BrushAssetPath);
		if (BrushAsset && BrushAsset->IsA<UTexture2D>())
		{
			UTexture2D* Texture = Cast<UTexture2D>(BrushAsset);
			FSlateBrush Brush;
			Brush.SetResourceObject(Texture);
			Brush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
			ImageWidget->SetBrush(Brush);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UMG: Failed to load texture asset from path: %s"), *BrushAssetPath);
		}
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(ImageWidget);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D ImageSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(ImageSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	if (!BrushAssetPath.IsEmpty())
	{
		Response->SetStringField(TEXT("brush_asset_path"), BrushAssetPath);
	}
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddCheckBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create CheckBox widget
	UCheckBox* CheckBox = WidgetBlueprint->WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), *WidgetName);
	if (!CheckBox)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create CheckBox widget"));
	}

	// Set initial checked state if provided
	bool IsChecked = false;
	Params->TryGetBoolField(TEXT("is_checked"), IsChecked);
	// Fixed to use bool instead of ECheckBoxState in UE 5.5
	CheckBox->SetIsChecked(IsChecked); 

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(CheckBox);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetBoolField(TEXT("is_checked"), IsChecked);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddSliderToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Slider widget
	USlider* Slider = WidgetBlueprint->WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), *WidgetName);
	if (!Slider)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Slider widget"));
	}

	// Set min value if provided
	float MinValue = 0.0f;
	Params->TryGetNumberField(TEXT("min_value"), MinValue);
	Slider->SetMinValue(MinValue);

	// Set max value if provided
	float MaxValue = 1.0f;
	Params->TryGetNumberField(TEXT("max_value"), MaxValue);
	Slider->SetMaxValue(MaxValue);

	// Set initial value if provided
	float Value = 0.5f;
	Params->TryGetNumberField(TEXT("value"), Value);
	Slider->SetValue(Value);

	// Set orientation if provided
	FString Orientation = TEXT("Horizontal");
	Params->TryGetStringField(TEXT("orientation"), Orientation);
	if (Orientation.Equals(TEXT("Vertical"), ESearchCase::IgnoreCase))
	{
		Slider->SetOrientation(EOrientation::Orient_Vertical);
	}
	else
	{
		Slider->SetOrientation(EOrientation::Orient_Horizontal);
	}

	// Set bar color if provided
	const TArray<TSharedPtr<FJsonValue>>* BarColorArray;
	if (Params->TryGetArrayField(TEXT("bar_color"), BarColorArray) && BarColorArray->Num() >= 4)
	{
		FLinearColor BarColor(
			(*BarColorArray)[0]->AsNumber(),
			(*BarColorArray)[1]->AsNumber(),
			(*BarColorArray)[2]->AsNumber(),
			(*BarColorArray)[3]->AsNumber()
		);
		
		// In UE 5.5 we can't set these colors directly on the slider
		// They need to be configured in the Widget Style
	}

	// Set handle color if provided
	const TArray<TSharedPtr<FJsonValue>>* HandleColorArray;
	if (Params->TryGetArrayField(TEXT("handle_color"), HandleColorArray) && HandleColorArray->Num() >= 4)
	{
		FLinearColor HandleColor(
			(*HandleColorArray)[0]->AsNumber(),
			(*HandleColorArray)[1]->AsNumber(),
			(*HandleColorArray)[2]->AsNumber(),
			(*HandleColorArray)[3]->AsNumber()
		);
		
		// In UE 5.5 we can't set these colors directly on the slider
		// They need to be configured in the Widget Style
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Slider);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D SliderSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(SliderSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetNumberField(TEXT("min_value"), MinValue);
	Response->SetNumberField(TEXT("max_value"), MaxValue);
	Response->SetNumberField(TEXT("value"), Value);
	Response->SetStringField(TEXT("orientation"), Orientation);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddProgressBarToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create ProgressBar widget
	UProgressBar* ProgressBar = WidgetBlueprint->WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), *WidgetName);
	if (!ProgressBar)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create ProgressBar widget"));
	}

	// Set percent if provided (default is 0.5)
	float Percent = 0.5f;
	Params->TryGetNumberField(TEXT("percent"), Percent);
	ProgressBar->SetPercent(Percent);

	// Set fill color if provided
	const TArray<TSharedPtr<FJsonValue>>* FillColorArray;
	if (Params->TryGetArrayField(TEXT("fill_color"), FillColorArray) && FillColorArray->Num() >= 4)
	{
		FLinearColor FillColor(
			(*FillColorArray)[0]->AsNumber(),
			(*FillColorArray)[1]->AsNumber(),
			(*FillColorArray)[2]->AsNumber(),
			(*FillColorArray)[3]->AsNumber()
		);
		ProgressBar->SetFillColorAndOpacity(FillColor);
	}

	// Set background color if provided - this requires using style object which isn't directly editable here
	const TArray<TSharedPtr<FJsonValue>>* BgColorArray;
	if (Params->TryGetArrayField(TEXT("background_color"), BgColorArray) && BgColorArray->Num() >= 4)
	{
		FLinearColor BgColor(
			(*BgColorArray)[0]->AsNumber(),
			(*BgColorArray)[1]->AsNumber(),
			(*BgColorArray)[2]->AsNumber(),
			(*BgColorArray)[3]->AsNumber()
		);
		// Note: In UE 5.5, we can't directly set the background color through code
		// This would require widget style customization in the design editor
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(ProgressBar);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D BarSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(BarSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetNumberField(TEXT("percent"), Percent);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddBorderToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Border widget
	UBorder* Border = WidgetBlueprint->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *WidgetName);
	if (!Border)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Border widget"));
	}

	// Set brush color if provided
	const TArray<TSharedPtr<FJsonValue>>* BrushColorArray;
	if (Params->TryGetArrayField(TEXT("brush_color"), BrushColorArray) && BrushColorArray->Num() >= 4)
	{
		FLinearColor BrushColor(
			(*BrushColorArray)[0]->AsNumber(),
			(*BrushColorArray)[1]->AsNumber(),
			(*BrushColorArray)[2]->AsNumber(),
			(*BrushColorArray)[3]->AsNumber()
		);
		Border->SetBrushColor(BrushColor);
	}

	// Set brush thickness if provided
	float BrushThickness = 4.0f;
	Params->TryGetNumberField(TEXT("brush_thickness"), BrushThickness);

	// Unfortunately UBorder doesn't have a direct way to set thickness
	// We would need to create a custom brush to achieve this effect
	
	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Border);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D BorderSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(BorderSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetNumberField(TEXT("brush_thickness"), BrushThickness);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddScrollBoxToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create ScrollBox widget
	UScrollBox* ScrollBox = WidgetBlueprint->WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), *WidgetName);
	if (!ScrollBox)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create ScrollBox widget"));
	}

	// Set orientation if provided
	FString Orientation = TEXT("Vertical");
	Params->TryGetStringField(TEXT("orientation"), Orientation);
	if (Orientation.Equals(TEXT("Horizontal"), ESearchCase::IgnoreCase))
	{
		ScrollBox->SetOrientation(EOrientation::Orient_Horizontal);
	}
	else
	{
		ScrollBox->SetOrientation(EOrientation::Orient_Vertical);
	}

	// Set scrollbar visibility if provided
	FString ScrollBarVisibility = TEXT("Visible");
	Params->TryGetStringField(TEXT("scroll_bar_visibility"), ScrollBarVisibility);
	
	if (ScrollBarVisibility.Equals(TEXT("Visible"), ESearchCase::IgnoreCase))
	{
		ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	}
	else if (ScrollBarVisibility.Equals(TEXT("Hidden"), ESearchCase::IgnoreCase))
	{
		ScrollBox->SetScrollBarVisibility(ESlateVisibility::Hidden);
	}
	else if (ScrollBarVisibility.Equals(TEXT("Auto"), ESearchCase::IgnoreCase))
	{
		// In UE5.5, there's no direct "Auto" visibility, we'll use this to simulate that behavior
		ScrollBox->SetScrollBarVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(ScrollBox);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D ScrollBoxSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(ScrollBoxSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	Response->SetStringField(TEXT("orientation"), Orientation);
	Response->SetStringField(TEXT("scroll_bar_visibility"), ScrollBarVisibility);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddSpacerToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Create Spacer widget
	USpacer* Spacer = WidgetBlueprint->WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), *WidgetName);
	if (!Spacer)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Spacer widget"));
	}

	// Get canvas panel and add widget
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position/size
	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(Spacer);
	if (PanelSlot)
	{
		// Set position if provided
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			PanelSlot->SetPosition(Pos);
		}

		// Set size if provided
		const TArray<TSharedPtr<FJsonValue>>* Size;
		if (Params->TryGetArrayField(TEXT("size"), Size) && Size->Num() >= 2)
		{
			FVector2D SpacerSize(
				(*Size)[0]->AsNumber(),
				(*Size)[1]->AsNumber()
			);
			PanelSlot->SetSize(SpacerSize);
		}
	}

	// Save the Widget Blueprint
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

	// Create success response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleCheckComponentExists(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString ComponentName;
	if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Check if the component exists
	UWidget* ExistingWidget = WidgetBlueprint->WidgetTree->FindWidget(*ComponentName);
	bool bExists = (ExistingWidget != nullptr);

	// Create response
	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("component_name"), ComponentName);
	Response->SetBoolField(TEXT("exists"), bExists);
	
	if (bExists)
	{
		// Include component class name if it exists
		Response->SetStringField(TEXT("component_type"), ExistingWidget->GetClass()->GetName());
	}
	
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddWidgetAsChild(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString ParentComponentName;
	if (!Params->TryGetStringField(TEXT("parent_component_name"), ParentComponentName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parent_component_name' parameter"));
	}

	FString ChildComponentName;
	if (!Params->TryGetStringField(TEXT("child_component_name"), ChildComponentName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'child_component_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Find the child widget
	UWidget* ChildWidget = WidgetBlueprint->WidgetTree->FindWidget(*ChildComponentName);
	if (!ChildWidget)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Child widget '%s' not found"), *ChildComponentName));
	}

	// Find the parent widget
	UWidget* ParentWidget = WidgetBlueprint->WidgetTree->FindWidget(*ParentComponentName);
	
	// Check if we need to create the parent
	bool CreateParentIfMissing = false;
	Params->TryGetBoolField(TEXT("create_parent_if_missing"), CreateParentIfMissing);
	
	if (!ParentWidget && CreateParentIfMissing)
	{
		// Get parent component type
		FString ParentComponentType = TEXT("Border");
		Params->TryGetStringField(TEXT("parent_component_type"), ParentComponentType);
		
		// Get position and size for new parent
		FVector2D Position(0.0f, 0.0f);
		if (Params->HasField(TEXT("parent_position")))
		{
			const TArray<TSharedPtr<FJsonValue>>* PosArray;
			if (Params->TryGetArrayField(TEXT("parent_position"), PosArray) && PosArray->Num() >= 2)
			{
				Position.X = (*PosArray)[0]->AsNumber();
				Position.Y = (*PosArray)[1]->AsNumber();
			}
		}
		
		FVector2D Size(300.0f, 200.0f);
		if (Params->HasField(TEXT("parent_size")))
		{
			const TArray<TSharedPtr<FJsonValue>>* SizeArray;
			if (Params->TryGetArrayField(TEXT("parent_size"), SizeArray) && SizeArray->Num() >= 2)
			{
				Size.X = (*SizeArray)[0]->AsNumber();
				Size.Y = (*SizeArray)[1]->AsNumber();
			}
		}
		
		// Create the parent based on the specified type
		if (ParentComponentType == TEXT("Border"))
		{
			UBorder* Border = WidgetBlueprint->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *ParentComponentName);
			ParentWidget = Border;
		}
		else if (ParentComponentType == TEXT("VerticalBox"))
		{
			UVerticalBox* VerticalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *ParentComponentName);
			ParentWidget = VerticalBox;
		}
		else if (ParentComponentType == TEXT("HorizontalBox"))
		{
			UHorizontalBox* HorizontalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *ParentComponentName);
			ParentWidget = HorizontalBox;
		}
		else if (ParentComponentType == TEXT("ScrollBox"))
		{
			UScrollBox* ScrollBox = WidgetBlueprint->WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), *ParentComponentName);
			ParentWidget = ScrollBox;
		}
		else if (ParentComponentType == TEXT("WidgetSwitcher"))
		{
			UWidgetSwitcher* Switcher = WidgetBlueprint->WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), *ParentComponentName);
			ParentWidget = Switcher;
		}
		else if (ParentComponentType == TEXT("Overlay"))
		{
			UOverlay* Overlay = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), *ParentComponentName);
			ParentWidget = Overlay;
		}
		else if (ParentComponentType == TEXT("SizeBox"))
		{
			USizeBox* SizeBox = WidgetBlueprint->WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *ParentComponentName);
			ParentWidget = SizeBox;
		}
		else
		{
			// Default to a Border if the parent component type is not recognized
			UBorder* Border = WidgetBlueprint->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *ParentComponentName);
			ParentWidget = Border;
		}
		
		// If we created a new parent widget, add it to the root canvas panel
		if (ParentWidget)
		{
			UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
			if (RootCanvas)
			{
				UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(ParentWidget);
				if (PanelSlot)
				{
					PanelSlot->SetPosition(Position);
					PanelSlot->SetSize(Size);
				}
			}
		}
	}
	
	// Check that we have a valid parent widget now
	if (!ParentWidget)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Parent widget '%s' not found and wasn't created"), *ParentComponentName));
	}
	
	// Check for circular reference - IMPORTANT: This prevents the recursion depth error
	// Check if the parent is a descendant of the child (which would create a cycle)
	UWidget* TestWidget = ParentWidget;
	while (TestWidget)
	{
		if (TestWidget == ChildWidget)
		{
			// Circular reference detected
			return FUnrealMCPCommonUtils::CreateErrorResponse(
				FString::Printf(TEXT("Cannot add '%s' as child of '%s' because it would create a circular reference"),
				*ChildComponentName, *ParentComponentName));
		}
		
		UPanelWidget* TestParent = TestWidget->GetParent();
		if (TestParent)
		{
			TestWidget = TestParent;
		}
		else
		{
			break;
		}
	}
	
	// Get child's current parent if any
	UPanelWidget* CurrentParent = ChildWidget->GetParent();
	if (CurrentParent)
	{
		// Remove from current parent first (this prevents crashes)
		CurrentParent->RemoveChild(ChildWidget);
	}
	
	// Add child to the new parent
	UPanelWidget* ParentPanel = Cast<UPanelWidget>(ParentWidget);
	if (ParentPanel)
	{
		ParentPanel->AddChild(ChildWidget);
		
		// Save the Widget Blueprint
		WidgetBlueprint->MarkPackageDirty();
		FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
		UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);
		
		// Create success response
		Response->SetBoolField(TEXT("success"), true);
		Response->SetStringField(TEXT("parent_component_name"), ParentComponentName);
		Response->SetStringField(TEXT("child_component_name"), ChildComponentName);
		return Response;
	}
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Parent widget '%s' is not a panel widget that can have children"), *ParentComponentName));
	}
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleCreateWidgetComponentWithChild(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString ParentComponentName;
	if (!Params->TryGetStringField(TEXT("parent_component_name"), ParentComponentName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parent_component_name' parameter"));
	}

	FString ChildComponentName;
	if (!Params->TryGetStringField(TEXT("child_component_name"), ChildComponentName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'child_component_name' parameter"));
	}

	// Load the Widget Blueprint using our helper function
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get parent and child component types
	FString ParentComponentType = TEXT("Border");
	Params->TryGetStringField(TEXT("parent_component_type"), ParentComponentType);
	
	FString ChildComponentType = TEXT("TextBlock");
	Params->TryGetStringField(TEXT("child_component_type"), ChildComponentType);
	
	// Get position and size for parent
	FVector2D Position(0.0f, 0.0f);
	if (Params->HasField(TEXT("parent_position")))
	{
		const TArray<TSharedPtr<FJsonValue>>* PosArray;
		if (Params->TryGetArrayField(TEXT("parent_position"), PosArray) && PosArray->Num() >= 2)
		{
			Position.X = (*PosArray)[0]->AsNumber();
			Position.Y = (*PosArray)[1]->AsNumber();
		}
	}
	
	FVector2D Size(300.0f, 200.0f);
	if (Params->HasField(TEXT("parent_size")))
	{
		const TArray<TSharedPtr<FJsonValue>>* SizeArray;
		if (Params->TryGetArrayField(TEXT("parent_size"), SizeArray) && SizeArray->Num() >= 2)
		{
			Size.X = (*SizeArray)[0]->AsNumber();
			Size.Y = (*SizeArray)[1]->AsNumber();
		}
	}
	
	// Check if any components with these names already exist
	UWidget* ExistingParent = WidgetBlueprint->WidgetTree->FindWidget(*ParentComponentName);
	if (ExistingParent)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("A widget named '%s' already exists"), *ParentComponentName));
	}
	
	UWidget* ExistingChild = WidgetBlueprint->WidgetTree->FindWidget(*ChildComponentName);
	if (ExistingChild)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("A widget named '%s' already exists"), *ChildComponentName));
	}
	
	// Create the parent widget based on the specified type
	UPanelWidget* ParentWidget = nullptr;
	
	if (ParentComponentType == TEXT("Border"))
	{
		UBorder* Border = WidgetBlueprint->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *ParentComponentName);
		ParentWidget = Border;
	}
	else if (ParentComponentType == TEXT("VerticalBox"))
	{
		UVerticalBox* VerticalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *ParentComponentName);
		ParentWidget = VerticalBox;
	}
	else if (ParentComponentType == TEXT("HorizontalBox"))
	{
		UHorizontalBox* HorizontalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *ParentComponentName);
		ParentWidget = HorizontalBox;
	}
	else if (ParentComponentType == TEXT("ScrollBox"))
	{
		UScrollBox* ScrollBox = WidgetBlueprint->WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), *ParentComponentName);
		ParentWidget = ScrollBox;
	}
	else if (ParentComponentType == TEXT("WidgetSwitcher"))
	{
		UWidgetSwitcher* Switcher = WidgetBlueprint->WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), *ParentComponentName);
		ParentWidget = Switcher;
	}
	else if (ParentComponentType == TEXT("Overlay"))
	{
		UOverlay* Overlay = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), *ParentComponentName);
		ParentWidget = Overlay;
	}
	else if (ParentComponentType == TEXT("SizeBox"))
	{
		USizeBox* SizeBox = WidgetBlueprint->WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *ParentComponentName);
		ParentWidget = SizeBox;
	}
	else
	{
		// Default to a Border if the parent component type is not recognized
		UBorder* Border = WidgetBlueprint->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *ParentComponentName);
		ParentWidget = Border;
	}
	
	// Create the child widget based on the specified type
	UWidget* ChildWidget = nullptr;
	
	if (ChildComponentType == TEXT("TextBlock"))
	{
		UTextBlock* TextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *ChildComponentName);
		ChildWidget = TextBlock;
		
		// Check for text attribute
		const TSharedPtr<FJsonObject>* ChildAttributes;
		if (Params->TryGetObjectField(TEXT("child_attributes"), ChildAttributes))
		{
			FString Text;
			if ((*ChildAttributes)->TryGetStringField(TEXT("text"), Text))
			{
				TextBlock->SetText(FText::FromString(Text));
			}
			
			int32 FontSize = 12;
			(*ChildAttributes)->TryGetNumberField(TEXT("font_size"), FontSize);
			
			// Set font size property if provided
			// In UE 5.5, we can't directly set the font size with a simple method,
			// but for a complete implementation we would need to use FSlateFontInfo
		}
	}
	else if (ChildComponentType == TEXT("Button"))
	{
		UButton* Button = WidgetBlueprint->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *ChildComponentName);
		ChildWidget = Button;
		
		// Check if we need to add text to the button
		const TSharedPtr<FJsonObject>* ChildAttributes;
		if (Params->TryGetObjectField(TEXT("child_attributes"), ChildAttributes))
		{
			FString ButtonText;
			if ((*ChildAttributes)->TryGetStringField(TEXT("text"), ButtonText))
			{
				// Create text block for the button
				UTextBlock* ButtonTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(
					UTextBlock::StaticClass(), 
					*(ChildComponentName + TEXT("_Text"))
				);
				
				if (ButtonTextBlock)
				{
					ButtonTextBlock->SetText(FText::FromString(ButtonText));
					Button->AddChild(ButtonTextBlock);
				}
			}
		}
	}
	else if (ChildComponentType == TEXT("Image"))
	{
		UImage* Image = WidgetBlueprint->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *ChildComponentName);
		ChildWidget = Image;
		
		// Check for brush asset path
		const TSharedPtr<FJsonObject>* ChildAttributes;
		if (Params->TryGetObjectField(TEXT("child_attributes"), ChildAttributes))
		{
			FString BrushAssetPath;
			if ((*ChildAttributes)->TryGetStringField(TEXT("brush_asset_path"), BrushAssetPath) && !BrushAssetPath.IsEmpty())
			{
				UObject* BrushAsset = UEditorAssetLibrary::LoadAsset(BrushAssetPath);
				if (BrushAsset && BrushAsset->IsA<UTexture2D>())
				{
					UTexture2D* Texture = Cast<UTexture2D>(BrushAsset);
					FSlateBrush Brush;
					Brush.SetResourceObject(Texture);
					Brush.ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
					Image->SetBrush(Brush);
				}
			}
		}
	}
	else if (ChildComponentType == TEXT("VerticalBox"))
	{
		UVerticalBox* VerticalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *ChildComponentName);
		ChildWidget = VerticalBox;
	}
	else if (ChildComponentType == TEXT("HorizontalBox"))
	{
		UHorizontalBox* HorizontalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *ChildComponentName);
		ChildWidget = HorizontalBox;
	}
	else
	{
		// Default to a TextBlock if the child component type is not recognized or implemented
		UTextBlock* TextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *ChildComponentName);
		ChildWidget = TextBlock;
		TextBlock->SetText(FText::FromString(TEXT("Default Text")));
	}
	
	// Add the parent to the root canvas panel
	if (ParentWidget)
	{
		UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
		if (RootCanvas)
		{
			UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(ParentWidget);
			if (PanelSlot)
			{
				PanelSlot->SetPosition(Position);
				PanelSlot->SetSize(Size);
			}
		}
	}
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create parent widget"));
	}
	
	// Add the child to the parent
	if (ChildWidget && ParentWidget)
	{
		ParentWidget->AddChild(ChildWidget);
		
		// Save the Widget Blueprint
		WidgetBlueprint->MarkPackageDirty();
		FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
		UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);
		
		// Create success response
		Response->SetBoolField(TEXT("success"), true);
		Response->SetStringField(TEXT("parent_component_name"), ParentComponentName);
		Response->SetStringField(TEXT("child_component_name"), ChildComponentName);
		Response->SetStringField(TEXT("parent_component_type"), ParentComponentType);
		Response->SetStringField(TEXT("child_component_type"), ChildComponentType);
		return Response;
	}
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create child widget or add it to parent"));
	}
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleSetWidgetPlacement(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString WidgetName;
    if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing required parameter: widget_name"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing required parameter: component_name"));
    }

    // Find the widget blueprint
    UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(WidgetName);
    if (!WidgetBlueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(*FString::Printf(TEXT("Widget blueprint not found: %s"), *WidgetName));
    }

    // Find the specified widget
    UWidget* TargetWidget = nullptr;
    
    // Only search if we have a valid widget tree
    if (WidgetBlueprint->WidgetTree)
    {
        WidgetBlueprint->WidgetTree->ForEachWidget([&TargetWidget, &ComponentName](UWidget* Widget) {
            if (Widget && Widget->GetName() == ComponentName)
            {
                TargetWidget = Widget;
                // Early out once we've found it
                return false;
            }
            return true;
        });
    }

    if (!TargetWidget)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(*FString::Printf(TEXT("Widget component not found: %s"), *ComponentName));
    }

    // Get the parent slot (likely a Canvas Panel Slot)
    UPanelSlot* ParentSlot = TargetWidget->Slot;
    UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ParentSlot);
    
    if (!CanvasSlot)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Widget is not in a Canvas Panel or doesn't have a valid slot"));
    }

    // Get optional parameters
    const TArray<TSharedPtr<FJsonValue>>* PositionArray;
    if (Params->TryGetArrayField(TEXT("position"), PositionArray) && PositionArray->Num() == 2)
    {
        float X = (*PositionArray)[0]->AsNumber();
        float Y = (*PositionArray)[1]->AsNumber();
        CanvasSlot->SetPosition(FVector2D(X, Y));
    }

    const TArray<TSharedPtr<FJsonValue>>* SizeArray;
    if (Params->TryGetArrayField(TEXT("size"), SizeArray) && SizeArray->Num() == 2)
    {
        float Width = (*SizeArray)[0]->AsNumber();
        float Height = (*SizeArray)[1]->AsNumber();
        CanvasSlot->SetSize(FVector2D(Width, Height));
    }

    const TArray<TSharedPtr<FJsonValue>>* AlignmentArray;
    if (Params->TryGetArrayField(TEXT("alignment"), AlignmentArray) && AlignmentArray->Num() == 2)
    {
        float HorizontalAlignment = (*AlignmentArray)[0]->AsNumber();
        float VerticalAlignment = (*AlignmentArray)[1]->AsNumber();
        CanvasSlot->SetAlignment(FVector2D(HorizontalAlignment, VerticalAlignment));
    }

    // Save the Widget Blueprint
    WidgetBlueprint->MarkPackageDirty();
    FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
    UEditorAssetLibrary::SaveAsset(WidgetBlueprint->GetPathName(), false);

    // Create success response with current values
    TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
    Response->SetBoolField(TEXT("success"), true);
    Response->SetStringField(TEXT("widget_name"), ComponentName);

    // Include current position in response
    TArray<TSharedPtr<FJsonValue>> CurrentPosition;
    CurrentPosition.Add(MakeShared<FJsonValueNumber>(CanvasSlot->GetPosition().X));
    CurrentPosition.Add(MakeShared<FJsonValueNumber>(CanvasSlot->GetPosition().Y));
    Response->SetArrayField(TEXT("position"), CurrentPosition);

    // Include current size in response
    TArray<TSharedPtr<FJsonValue>> CurrentSize;
    CurrentSize.Add(MakeShared<FJsonValueNumber>(CanvasSlot->GetSize().X));
    CurrentSize.Add(MakeShared<FJsonValueNumber>(CanvasSlot->GetSize().Y));
    Response->SetArrayField(TEXT("size"), CurrentSize);

    // Include current alignment in response
    TArray<TSharedPtr<FJsonValue>> CurrentAlignment;
    CurrentAlignment.Add(MakeShared<FJsonValueNumber>(CanvasSlot->GetAlignment().X));
    CurrentAlignment.Add(MakeShared<FJsonValueNumber>(CanvasSlot->GetAlignment().Y));
    Response->SetArrayField(TEXT("alignment"), CurrentAlignment);

    return Response;
}

// Helper function to create error response
TSharedPtr<FJsonObject> CreateErrorResponse(const FString& ErrorMessage)
{
    TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();
    Response->SetBoolField(TEXT("success"), false);
    Response->SetStringField(TEXT("error"), ErrorMessage);
    return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleGetWidgetContainerDimensions(const TSharedPtr<FJsonObject>& InJson)
{
    // Get the widget name and container name from the input JSON
    FString WidgetName;
    FString ContainerName;
    if (!InJson->TryGetStringField(TEXT("widget_name"), WidgetName) ||
        !InJson->TryGetStringField(TEXT("container_name"), ContainerName))
    {
        return MakeShared<FJsonObject>();
    }

    // Find the widget blueprint
    UWidgetBlueprint* WidgetBP = FindWidgetBlueprint(WidgetName);
    if (!WidgetBP || !WidgetBP->WidgetTree)
    {
        return CreateErrorResponse(TEXT("Widget not found"));
    }

    // Find the container widget
    UWidget* ContainerWidget = WidgetBP->WidgetTree->FindWidget(FName(*ContainerName));
    if (!ContainerWidget)
    {
        return CreateErrorResponse(TEXT("Container widget not found: ") + ContainerName);
    }

    // Create response JSON
    TSharedPtr<FJsonObject> ResponseJson = MakeShared<FJsonObject>();
    
    // Force layout prepass to ensure sizes are computed
    ContainerWidget->ForceLayoutPrepass();

    // Get the desired size of the container
    FVector2D DesiredSize = ContainerWidget->GetDesiredSize();
    
    // Try to get dimensions from canvas panel slot
    if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ContainerWidget->Slot))
    {
        FVector2D Position = CanvasSlot->GetPosition();
        FVector2D Size = CanvasSlot->GetSize();
        
        ResponseJson->SetNumberField(TEXT("x"), Position.X);
        ResponseJson->SetNumberField(TEXT("y"), Position.Y);
        ResponseJson->SetNumberField(TEXT("width"), Size.X);
        ResponseJson->SetNumberField(TEXT("height"), Size.Y);
        ResponseJson->SetBoolField(TEXT("from_slot"), true);
        
        // Also include the desired size
        ResponseJson->SetNumberField(TEXT("desired_width"), DesiredSize.X);
        ResponseJson->SetNumberField(TEXT("desired_height"), DesiredSize.Y);
    }
    else
    {
        // No canvas slot, use desired size
        ResponseJson->SetNumberField(TEXT("x"), 0);
        ResponseJson->SetNumberField(TEXT("y"), 0);
        ResponseJson->SetNumberField(TEXT("width"), DesiredSize.X);
        ResponseJson->SetNumberField(TEXT("height"), DesiredSize.Y);
        ResponseJson->SetBoolField(TEXT("from_slot"), false);
    }

    // Get parent widget dimensions
    UWidget* ParentWidget = ContainerWidget->GetParent();
    if (ParentWidget)
    {
        // Force layout prepass on parent
        ParentWidget->ForceLayoutPrepass();
        FVector2D ParentSize = ParentWidget->GetDesiredSize();
        
        // If parent is a canvas panel, use its actual size
        if (UCanvasPanel* ParentCanvas = Cast<UCanvasPanel>(ParentWidget))
        {
            if (UCanvasPanelSlot* ParentSlot = Cast<UCanvasPanelSlot>(ParentCanvas->Slot))
            {
                ParentSize = ParentSlot->GetSize();
            }
        }
        
        ResponseJson->SetNumberField(TEXT("parent_width"), ParentSize.X);
        ResponseJson->SetNumberField(TEXT("parent_height"), ParentSize.Y);
        ResponseJson->SetBoolField(TEXT("has_parent_size"), true);
    }
    else 
    {
        ResponseJson->SetBoolField(TEXT("has_parent_size"), false);
    }

    // Get viewport size if this is the root widget
    if (ContainerName.Equals(TEXT("RootCanvas")))
    {
        if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
        {
            FIntPoint ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
            ResponseJson->SetNumberField(TEXT("viewport_width"), ViewportSize.X);
            ResponseJson->SetNumberField(TEXT("viewport_height"), ViewportSize.Y);
            ResponseJson->SetBoolField(TEXT("has_viewport_size"), true);
        }
        else
        {
            ResponseJson->SetBoolField(TEXT("has_viewport_size"), false);
        }
    }

    ResponseJson->SetBoolField(TEXT("success"), true);
    return ResponseJson;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddWidgetComponent(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString ComponentName;
	if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
	}

	FString ComponentType;
	if (!Params->TryGetStringField(TEXT("component_type"), ComponentType))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_type' parameter"));
	}

	// Find the Widget Blueprint
	UWidgetBlueprint* WidgetBlueprint = FindWidgetBlueprint(BlueprintName);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get position parameter if it exists
	FVector2D Position(0.0f, 0.0f);
	if (Params->HasField(TEXT("position")))
	{
		TArray<TSharedPtr<FJsonValue>> PosArray;
		if (GetJsonArray(Params, TEXT("position"), PosArray) && PosArray.Num() >= 2)
		{
			Position.X = PosArray[0]->AsNumber();
			Position.Y = PosArray[1]->AsNumber();
		}
	}

	// Get size parameter if it exists
	FVector2D Size(100.0f, 100.0f);
	if (Params->HasField(TEXT("size")))
	{
		TArray<TSharedPtr<FJsonValue>> SizeArray;
		if (GetJsonArray(Params, TEXT("size"), SizeArray) && SizeArray.Num() >= 2)
		{
			Size.X = SizeArray[0]->AsNumber();
			Size.Y = SizeArray[1]->AsNumber();
		}
	}

	// Get kwargs object if it exists
	const TSharedPtr<FJsonObject>* KwargsObject;
	TSharedPtr<FJsonObject> KwargsObjectRef;
	if (Params->TryGetObjectField(TEXT("kwargs"), KwargsObject))
	{
		KwargsObjectRef = *KwargsObject;
	}
	else
	{
		KwargsObjectRef = MakeShared<FJsonObject>(); // Create empty object if not present
	}

	// Create the appropriate widget based on component type
	UWidget* CreatedWidget = nullptr;

	// TextBlock
	if (ComponentType.Equals(TEXT("TextBlock"), ESearchCase::IgnoreCase))
	{
		UTextBlock* TextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *ComponentName);
		CreatedWidget = TextBlock;
		
		// Apply text block specific properties
		FString Text;
		if (KwargsObjectRef->TryGetStringField(TEXT("text"), Text))
		{
			TextBlock->SetText(FText::FromString(Text));
		}
		
		// Apply font size if provided
		int32 FontSize = 12;
		KwargsObjectRef->TryGetNumberField(TEXT("font_size"), FontSize);
		
		// Apply text color if provided
		TArray<TSharedPtr<FJsonValue>> ColorArray;
		if (GetJsonArray(KwargsObjectRef, TEXT("color"), ColorArray) && ColorArray.Num() >= 3)
		{
			float R = ColorArray[0]->AsNumber();
			float G = ColorArray[1]->AsNumber();
			float B = ColorArray[2]->AsNumber();
			float A = ColorArray.Num() >= 4 ? ColorArray[3]->AsNumber() : 1.0f;
			
			FSlateColor TextColor(FLinearColor(R, G, B, A));
			TextBlock->SetColorAndOpacity(TextColor);
		}
	}
	// Button
	else if (ComponentType.Equals(TEXT("Button"), ESearchCase::IgnoreCase))
	{
		UButton* Button = WidgetBlueprint->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *ComponentName);
		CreatedWidget = Button;
		
		// Apply button specific properties
		FString ButtonText;
		if (KwargsObjectRef->TryGetStringField(TEXT("text"), ButtonText))
		{
			// Create text block for the button
			UTextBlock* ButtonTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(
				UTextBlock::StaticClass(), 
				*(ComponentName + TEXT("_Text"))
			);
			
			if (ButtonTextBlock)
			{
				ButtonTextBlock->SetText(FText::FromString(ButtonText));
				
				// Apply font size to button text if provided
				int32 FontSize = 12;
				KwargsObjectRef->TryGetNumberField(TEXT("font_size"), FontSize);
				
				// Apply text color to button text if provided
				TArray<TSharedPtr<FJsonValue>> ColorArray;
				if (GetJsonArray(KwargsObjectRef, TEXT("color"), ColorArray) && ColorArray.Num() >= 3)
				{
					float R = ColorArray[0]->AsNumber();
					float G = ColorArray[1]->AsNumber();
					float B = ColorArray[2]->AsNumber();
					float A = ColorArray.Num() >= 4 ? ColorArray[3]->AsNumber() : 1.0f;
					
					FSlateColor TextColor(FLinearColor(R, G, B, A));
					ButtonTextBlock->SetColorAndOpacity(TextColor);
				}
				
				Button->AddChild(ButtonTextBlock);
			}
		}
		
		// Apply background color if provided
		TArray<TSharedPtr<FJsonValue>> BgColorArray;
		if (GetJsonArray(KwargsObjectRef, TEXT("background_color"), BgColorArray) && BgColorArray.Num() >= 3)
		{
			float R = BgColorArray[0]->AsNumber();
			float G = BgColorArray[1]->AsNumber();
			float B = BgColorArray[2]->AsNumber();
			float A = BgColorArray.Num() >= 4 ? BgColorArray[3]->AsNumber() : 1.0f;
			
			// This is a simplified approach; in a full implementation you would
			// use UButton's style properties to set the background color
		}
	}
	// Image
	else if (ComponentType.Equals(TEXT("Image"), ESearchCase::IgnoreCase))
	{
		UImage* Image = WidgetBlueprint->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *ComponentName);
		CreatedWidget = Image;
		
		// Apply image specific properties
		FString BrushAssetPath;
		if (KwargsObjectRef->TryGetStringField(TEXT("brush_asset_path"), BrushAssetPath) && !BrushAssetPath.IsEmpty())
		{
			// Set the brush asset if provided
			// In a full implementation, you would load the asset and set it as the image brush
		}
	}
	// CheckBox
	else if (ComponentType.Equals(TEXT("CheckBox"), ESearchCase::IgnoreCase))
	{
		UCheckBox* CheckBox = WidgetBlueprint->WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), *ComponentName);
		CreatedWidget = CheckBox;
		
		// Apply checkbox specific properties
		bool IsChecked = false;
		if (KwargsObjectRef->TryGetBoolField(TEXT("is_checked"), IsChecked))
		{
			CheckBox->SetIsChecked(IsChecked);
		}
	}
	// Slider
	else if (ComponentType.Equals(TEXT("Slider"), ESearchCase::IgnoreCase))
	{
		USlider* Slider = WidgetBlueprint->WidgetTree->ConstructWidget<USlider>(USlider::StaticClass(), *ComponentName);
		CreatedWidget = Slider;
		
		// Apply slider specific properties
		float MinValue = 0.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("min_value"), MinValue);
		Slider->SetMinValue(MinValue);
		
		float MaxValue = 1.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("max_value"), MaxValue);
		Slider->SetMaxValue(MaxValue);
		
		float Value = 0.5f;
		KwargsObjectRef->TryGetNumberField(TEXT("value"), Value);
		Slider->SetValue(Value);
		
		FString Orientation;
		if (KwargsObjectRef->TryGetStringField(TEXT("orientation"), Orientation))
		{
			Slider->SetOrientation(Orientation.Equals(TEXT("Horizontal"), ESearchCase::IgnoreCase) 
				? Orient_Horizontal : Orient_Vertical);
		}
	}
	// ProgressBar
	else if (ComponentType.Equals(TEXT("ProgressBar"), ESearchCase::IgnoreCase))
	{
		UProgressBar* ProgressBar = WidgetBlueprint->WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), *ComponentName);
		CreatedWidget = ProgressBar;
		
		// Apply progress bar specific properties
		float Percent = 0.5f;
		KwargsObjectRef->TryGetNumberField(TEXT("percent"), Percent);
		ProgressBar->SetPercent(Percent);
		
		// Apply fill color if provided
		TArray<TSharedPtr<FJsonValue>> FillColorArray;
		if (GetJsonArray(KwargsObjectRef, TEXT("fill_color"), FillColorArray) && FillColorArray.Num() >= 3)
		{
			float R = FillColorArray[0]->AsNumber();
			float G = FillColorArray[1]->AsNumber();
			float B = FillColorArray[2]->AsNumber();
			float A = FillColorArray.Num() >= 4 ? FillColorArray[3]->AsNumber() : 1.0f;
			
			FLinearColor FillColor(R, G, B, A);
			ProgressBar->SetFillColorAndOpacity(FillColor);
		}
	}
	// Border
	else if (ComponentType.Equals(TEXT("Border"), ESearchCase::IgnoreCase))
	{
		UBorder* Border = WidgetBlueprint->WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *ComponentName);
		CreatedWidget = Border;
		
		// Apply border specific properties
		TArray<TSharedPtr<FJsonValue>> BrushColorArray;
		if (GetJsonArray(KwargsObjectRef, TEXT("brush_color"), BrushColorArray) && BrushColorArray.Num() >= 3)
		{
			float R = BrushColorArray[0]->AsNumber();
			float G = BrushColorArray[1]->AsNumber();
			float B = BrushColorArray[2]->AsNumber();
			float A = BrushColorArray.Num() >= 4 ? BrushColorArray[3]->AsNumber() : 1.0f;
			
			FLinearColor BrushColor(R, G, B, A);
			Border->SetBrushColor(BrushColor);
		}
	}
	// ScrollBox
	else if (ComponentType.Equals(TEXT("ScrollBox"), ESearchCase::IgnoreCase))
	{
		UScrollBox* ScrollBox = WidgetBlueprint->WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), *ComponentName);
		CreatedWidget = ScrollBox;
		
		// Apply scroll box specific properties
		FString Orientation;
		if (KwargsObjectRef->TryGetStringField(TEXT("orientation"), Orientation))
		{
			ScrollBox->SetOrientation(Orientation.Equals(TEXT("Horizontal"), ESearchCase::IgnoreCase) 
				? Orient_Horizontal : Orient_Vertical);
		}
		
		FString ScrollBarVisibility;
		if (KwargsObjectRef->TryGetStringField(TEXT("scroll_bar_visibility"), ScrollBarVisibility))
		{
			// Set scroll bar visibility based on string value
			// In a full implementation, you would map string values to ESlateVisibility enum
		}
	}
	// Spacer
	else if (ComponentType.Equals(TEXT("Spacer"), ESearchCase::IgnoreCase))
	{
		USpacer* Spacer = WidgetBlueprint->WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), *ComponentName);
		CreatedWidget = Spacer;
		// No special properties for spacer beyond size
	}
	// WidgetSwitcher
	else if (ComponentType.Equals(TEXT("WidgetSwitcher"), ESearchCase::IgnoreCase))
	{
		UWidgetSwitcher* Switcher = WidgetBlueprint->WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), *ComponentName);
		CreatedWidget = Switcher;
		
		// Apply widget switcher specific properties
		int32 ActiveIndex = 0;
		KwargsObjectRef->TryGetNumberField(TEXT("active_widget_index"), ActiveIndex);
		Switcher->SetActiveWidgetIndex(ActiveIndex);
	}
	// Throbber
	else if (ComponentType.Equals(TEXT("Throbber"), ESearchCase::IgnoreCase))
	{
		UThrobber* Throbber = WidgetBlueprint->WidgetTree->ConstructWidget<UThrobber>(UThrobber::StaticClass(), *ComponentName);
		CreatedWidget = Throbber;
		
		// Apply throbber specific properties
		int32 NumPieces = 3;
		KwargsObjectRef->TryGetNumberField(TEXT("num_pieces"), NumPieces);
		Throbber->SetNumberOfPieces(NumPieces);
		
		bool Animate = true;
		KwargsObjectRef->TryGetBoolField(TEXT("animate"), Animate);
		Throbber->SetAnimateHorizontally(Animate);
		Throbber->SetAnimateVertically(Animate);
	}
	// ExpandableArea
	else if (ComponentType.Equals(TEXT("ExpandableArea"), ESearchCase::IgnoreCase))
	{
		// Create the ExpandableArea without trying to set header content
		UExpandableArea* ExpandableArea = WidgetBlueprint->WidgetTree->ConstructWidget<UExpandableArea>(UExpandableArea::StaticClass(), *ComponentName);
		
		// Create a text block for the header text separately
		FString HeaderText;
		UTextBlock* HeaderTextBlock = nullptr;
		if (KwargsObjectRef->TryGetStringField(TEXT("header_text"), HeaderText))
		{
			// We'll create a separate text block with the header text
			HeaderTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(
				UTextBlock::StaticClass(), 
				*(ComponentName + TEXT("_HeaderText"))
			);
			HeaderTextBlock->SetText(FText::FromString(HeaderText));
		}
		
		// Set expansion state
		bool IsExpanded = false;
		KwargsObjectRef->TryGetBoolField(TEXT("is_expanded"), IsExpanded);
		ExpandableArea->SetIsExpanded(IsExpanded);
		
		// Use both widgets - let the user arrange them in the Blueprint editor
		// We won't try to set one as the header of the other
		CreatedWidget = ExpandableArea;
		
		// Just log that the user will need to set up the header content manually
		UE_LOG(LogTemp, Warning, TEXT("Created ExpandableArea '%s'. In UE 5.5, you'll need to manually set the header content in the Widget Blueprint."), *ComponentName);
	}
	// RichTextBlock
	else if (ComponentType.Equals(TEXT("RichTextBlock"), ESearchCase::IgnoreCase))
	{
		URichTextBlock* RichTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass(), *ComponentName);
		CreatedWidget = RichTextBlock;
		
		// Apply rich text block specific properties
		FString Text;
		if (KwargsObjectRef->TryGetStringField(TEXT("text"), Text))
		{
			RichTextBlock->SetText(FText::FromString(Text));
		}
		
		bool AutoWrapText = true;
		KwargsObjectRef->TryGetBoolField(TEXT("auto_wrap_text"), AutoWrapText);
		RichTextBlock->SetAutoWrapText(AutoWrapText);
	}
	// MultiLineEditableText
	else if (ComponentType.Equals(TEXT("MultiLineEditableText"), ESearchCase::IgnoreCase))
	{
		UMultiLineEditableText* TextBox = WidgetBlueprint->WidgetTree->ConstructWidget<UMultiLineEditableText>(UMultiLineEditableText::StaticClass(), *ComponentName);
		CreatedWidget = TextBox;
		
		// Apply text box specific properties
		FString Text;
		if (KwargsObjectRef->TryGetStringField(TEXT("text"), Text))
		{
			TextBox->SetText(FText::FromString(Text));
		}
		
		FString HintText;
		if (KwargsObjectRef->TryGetStringField(TEXT("hint_text"), HintText))
		{
			TextBox->SetHintText(FText::FromString(HintText));
		}
	}
	// Vertical Box
	else if (ComponentType.Equals(TEXT("VerticalBox"), ESearchCase::IgnoreCase))
	{
		UVerticalBox* VerticalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *ComponentName);
		CreatedWidget = VerticalBox;
		// No special properties for vertical box beyond children
	}
	// Horizontal Box
	else if (ComponentType.Equals(TEXT("HorizontalBox"), ESearchCase::IgnoreCase))
	{
		UHorizontalBox* HorizontalBox = WidgetBlueprint->WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *ComponentName);
		CreatedWidget = HorizontalBox;
		// No special properties for horizontal box beyond children
	}
	// Overlay
	else if (ComponentType.Equals(TEXT("Overlay"), ESearchCase::IgnoreCase))
	{
		UOverlay* Overlay = WidgetBlueprint->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), *ComponentName);
		CreatedWidget = Overlay;
		// No special properties for overlay beyond children
	}
	// GridPanel
	else if (ComponentType.Equals(TEXT("GridPanel"), ESearchCase::IgnoreCase))
	{
		UGridPanel* GridPanel = WidgetBlueprint->WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), *ComponentName);
		CreatedWidget = GridPanel;
		
		// Get column and row fill properties if provided
		int32 ColumnCount = 2;
		KwargsObjectRef->TryGetNumberField(TEXT("column_count"), ColumnCount);
		
		int32 RowCount = 2;
		KwargsObjectRef->TryGetNumberField(TEXT("row_count"), RowCount);
		
		// In a more complete implementation, you might set up initial columns/rows
		// but this requires more complex setup that's usually done when children are added
	}
	// SizeBox
	else if (ComponentType.Equals(TEXT("SizeBox"), ESearchCase::IgnoreCase))
	{
		USizeBox* SizeBox = WidgetBlueprint->WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *ComponentName);
		CreatedWidget = SizeBox;
		
		// Apply size box specific properties
		float MinWidth = 0.0f;
		if (KwargsObjectRef->TryGetNumberField(TEXT("min_width"), MinWidth) && MinWidth > 0.0f)
		{
			SizeBox->SetMinDesiredWidth(MinWidth);
		}
		
		float MinHeight = 0.0f;
		if (KwargsObjectRef->TryGetNumberField(TEXT("min_height"), MinHeight) && MinHeight > 0.0f)
		{
			SizeBox->SetMinDesiredHeight(MinHeight);
		}
		
		float MaxWidth = 0.0f;
		if (KwargsObjectRef->TryGetNumberField(TEXT("max_width"), MaxWidth) && MaxWidth > 0.0f)
		{
			SizeBox->SetMaxDesiredWidth(MaxWidth);
		}
		
		float MaxHeight = 0.0f;
		if (KwargsObjectRef->TryGetNumberField(TEXT("max_height"), MaxHeight) && MaxHeight > 0.0f)
		{
			SizeBox->SetMaxDesiredHeight(MaxHeight);
		}
	}
	// CanvasPanel
	else if (ComponentType.Equals(TEXT("CanvasPanel"), ESearchCase::IgnoreCase))
	{
		UCanvasPanel* CanvasPanel = WidgetBlueprint->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), *ComponentName);
		CreatedWidget = CanvasPanel;
		// No special properties for canvas panel beyond children
	}
	// ComboBox
	else if (ComponentType.Equals(TEXT("ComboBox"), ESearchCase::IgnoreCase))
	{
		UComboBoxString* ComboBox = WidgetBlueprint->WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), *ComponentName);
		CreatedWidget = ComboBox;
		
		// Set options if provided
		TArray<TSharedPtr<FJsonValue>> Options;
		if (GetJsonArray(KwargsObjectRef, TEXT("options"), Options))
		{
			for (const TSharedPtr<FJsonValue>& Option : Options)
			{
				FString OptionText = Option->AsString();
				ComboBox->AddOption(OptionText);
			}
		}
		
		// Default selected option
		FString DefaultOption;
		if (KwargsObjectRef->TryGetStringField(TEXT("default_option"), DefaultOption))
		{
			ComboBox->SetSelectedOption(DefaultOption);
		}
	}
	// EditableText (single line)
	else if (ComponentType.Equals(TEXT("EditableText"), ESearchCase::IgnoreCase))
	{
		UEditableText* TextEdit = WidgetBlueprint->WidgetTree->ConstructWidget<UEditableText>(UEditableText::StaticClass(), *ComponentName);
		CreatedWidget = TextEdit;
		
		// Apply editable text specific properties
		FString Text;
		if (KwargsObjectRef->TryGetStringField(TEXT("text"), Text))
		{
			TextEdit->SetText(FText::FromString(Text));
		}
		
		FString HintText;
		if (KwargsObjectRef->TryGetStringField(TEXT("hint_text"), HintText))
		{
			TextEdit->SetHintText(FText::FromString(HintText));
		}
		
		bool IsPassword = false;
		if (KwargsObjectRef->TryGetBoolField(TEXT("is_password"), IsPassword))
		{
			TextEdit->SetIsPassword(IsPassword);
		}
		
		bool IsReadOnly = false;
		if (KwargsObjectRef->TryGetBoolField(TEXT("is_read_only"), IsReadOnly))
		{
			TextEdit->SetIsReadOnly(IsReadOnly);
		}
	}
	// EditableTextBox (single line with styling)
	else if (ComponentType.Equals(TEXT("EditableTextBox"), ESearchCase::IgnoreCase))
	{
		UEditableTextBox* TextBox = WidgetBlueprint->WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), *ComponentName);
		CreatedWidget = TextBox;
		
		// Apply editable text box specific properties
		FString Text;
		if (KwargsObjectRef->TryGetStringField(TEXT("text"), Text))
		{
			TextBox->SetText(FText::FromString(Text));
		}
		
		FString HintText;
		if (KwargsObjectRef->TryGetStringField(TEXT("hint_text"), HintText))
		{
			TextBox->SetHintText(FText::FromString(HintText));
		}
		
		bool IsPassword = false;
		if (KwargsObjectRef->TryGetBoolField(TEXT("is_password"), IsPassword))
		{
			TextBox->SetIsPassword(IsPassword);
		}
		
		bool IsReadOnly = false;
		if (KwargsObjectRef->TryGetBoolField(TEXT("is_read_only"), IsReadOnly))
		{
			TextBox->SetIsReadOnly(IsReadOnly);
		}
	}
	// CircularThrobber
	else if (ComponentType.Equals(TEXT("CircularThrobber"), ESearchCase::IgnoreCase))
	{
		UCircularThrobber* Throbber = WidgetBlueprint->WidgetTree->ConstructWidget<UCircularThrobber>(UCircularThrobber::StaticClass(), *ComponentName);
		CreatedWidget = Throbber;
		
		// Apply circular throbber specific properties
		int32 NumPieces = 8;
		KwargsObjectRef->TryGetNumberField(TEXT("num_pieces"), NumPieces);
		Throbber->SetNumberOfPieces(NumPieces);
		
		float Period = 0.75f;
		KwargsObjectRef->TryGetNumberField(TEXT("period"), Period);
		Throbber->SetPeriod(Period);
		
		float Radius = 16.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("radius"), Radius);
		Throbber->SetRadius(Radius);
	}
	// SpinBox
	else if (ComponentType.Equals(TEXT("SpinBox"), ESearchCase::IgnoreCase))
	{
		USpinBox* SpinBox = WidgetBlueprint->WidgetTree->ConstructWidget<USpinBox>(USpinBox::StaticClass(), *ComponentName);
		CreatedWidget = SpinBox;
		
		// Apply spin box specific properties
		float MinValue = 0.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("min_value"), MinValue);
		SpinBox->SetMinValue(MinValue);
		
		float MaxValue = 100.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("max_value"), MaxValue);
		SpinBox->SetMaxValue(MaxValue);
		
		float Value = 0.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("value"), Value);
		SpinBox->SetValue(Value);
		
		float StepSize = 1.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("step_size"), StepSize);
		SpinBox->SetMinSliderValue(StepSize);
	}
	// WrapBox
	else if (ComponentType.Equals(TEXT("WrapBox"), ESearchCase::IgnoreCase))
	{
		UWrapBox* WrapBox = WidgetBlueprint->WidgetTree->ConstructWidget<UWrapBox>(UWrapBox::StaticClass(), *ComponentName);
		CreatedWidget = WrapBox;
		
		// Apply wrap box specific properties
		float WrapWidth = 500.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("wrap_width"), WrapWidth);
		// Note: In UE5.5, SetWrapWidth is not available - wrap width needs to be configured in the Widget Editor
		
		// Apply wrap horizontal/vertical alignment
		FString HorizontalAlignment;
		if (KwargsObjectRef->TryGetStringField(TEXT("horizontal_alignment"), HorizontalAlignment))
		{
			if (HorizontalAlignment.Equals(TEXT("Left"), ESearchCase::IgnoreCase))
				WrapBox->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
			else if (HorizontalAlignment.Equals(TEXT("Center"), ESearchCase::IgnoreCase))
				WrapBox->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			else if (HorizontalAlignment.Equals(TEXT("Right"), ESearchCase::IgnoreCase))
				WrapBox->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		}
	}
	// ScaleBox
	else if (ComponentType.Equals(TEXT("ScaleBox"), ESearchCase::IgnoreCase))
	{
		UScaleBox* ScaleBox = WidgetBlueprint->WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), *ComponentName);
		CreatedWidget = ScaleBox;
		
		// Apply scale box specific properties
		FString StretchDirection;
		if (KwargsObjectRef->TryGetStringField(TEXT("stretch_direction"), StretchDirection))
		{
			if (StretchDirection.Equals(TEXT("Both"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretchDirection(EStretchDirection::Both);
			else if (StretchDirection.Equals(TEXT("DownOnly"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretchDirection(EStretchDirection::DownOnly);
			else if (StretchDirection.Equals(TEXT("UpOnly"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretchDirection(EStretchDirection::UpOnly);
		}
		
		FString Stretch;
		if (KwargsObjectRef->TryGetStringField(TEXT("stretch"), Stretch))
		{
			if (Stretch.Equals(TEXT("None"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretch(EStretch::None);
			else if (Stretch.Equals(TEXT("Fill"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretch(EStretch::Fill);
			else if (Stretch.Equals(TEXT("ScaleToFit"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretch(EStretch::ScaleToFit);
			else if (Stretch.Equals(TEXT("ScaleToFitX"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretch(EStretch::ScaleToFitX);
			else if (Stretch.Equals(TEXT("ScaleToFitY"), ESearchCase::IgnoreCase))
				ScaleBox->SetStretch(EStretch::ScaleToFitY);
		}
		
		float UserSpecifiedScale = 1.0f;
		if (KwargsObjectRef->TryGetNumberField(TEXT("scale"), UserSpecifiedScale))
		{
			ScaleBox->SetUserSpecifiedScale(UserSpecifiedScale);
		}
	}
	// NamedSlot
	else if (ComponentType.Equals(TEXT("NamedSlot"), ESearchCase::IgnoreCase))
	{
		UNamedSlot* NamedSlot = WidgetBlueprint->WidgetTree->ConstructWidget<UNamedSlot>(UNamedSlot::StaticClass(), *ComponentName);
		CreatedWidget = NamedSlot;
		// No special properties for named slot beyond standard widget properties
	}
	// RadialSlider
	else if (ComponentType.Equals(TEXT("RadialSlider"), ESearchCase::IgnoreCase))
	{
		URadialSlider* RadialSlider = WidgetBlueprint->WidgetTree->ConstructWidget<URadialSlider>(URadialSlider::StaticClass(), *ComponentName);
		CreatedWidget = RadialSlider;
		
		// Apply radial slider specific properties
		float Value = 0.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("value"), Value);
		RadialSlider->SetValue(Value);
		
		float MinValue = 0.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("min_value"), MinValue);
		// RadialSlider->MinValue = MinValue; // Directly setting the member variable - Caused compilation error
		UE_LOG(LogTemp, Warning, TEXT("URadialSlider: MinValue cannot be set directly via code in this UE version. Please set it in the Widget Blueprint editor."));
		
		float MaxValue = 1.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("max_value"), MaxValue);
		// RadialSlider->MaxValue = MaxValue; // Directly setting the member variable - Caused compilation error
		UE_LOG(LogTemp, Warning, TEXT("URadialSlider: MaxValue cannot be set directly via code in this UE version. Please set it in the Widget Blueprint editor."));
		
		// Set slider colors if provided
		TArray<TSharedPtr<FJsonValue>> SliderBarColorArray;
		if (GetJsonArray(KwargsObjectRef, TEXT("slider_bar_color"), SliderBarColorArray) && SliderBarColorArray.Num() >= 3)
		{
			float R = SliderBarColorArray[0]->AsNumber();
			float G = SliderBarColorArray[1]->AsNumber();
			float B = SliderBarColorArray[2]->AsNumber();
			float A = SliderBarColorArray.Num() >= 4 ? SliderBarColorArray[3]->AsNumber() : 1.0f;
			
			FLinearColor SliderBarColor(R, G, B, A);
			// RadialSlider doesn't have a SetSliderBarColor method in UE 5.5
			// We'd need to set widget style properties to change colors
		}
	}
	// TextBox (single line text box with styling)
	else if (ComponentType.Equals(TEXT("TextBox"), ESearchCase::IgnoreCase))
	{
		// Use EditableTextBox directly, since TextBox isn't a standard UE5 component
		UEditableTextBox* TextBox = WidgetBlueprint->WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), *ComponentName);
		CreatedWidget = TextBox;
		
		// Apply text box specific properties
		FString Text;
		if (KwargsObjectRef->TryGetStringField(TEXT("text"), Text))
		{
			TextBox->SetText(FText::FromString(Text));
		}
		
		FString HintText;
		if (KwargsObjectRef->TryGetStringField(TEXT("hint_text"), HintText))
		{
			TextBox->SetHintText(FText::FromString(HintText));
		}
		
		bool IsReadOnly = false;
		KwargsObjectRef->TryGetBoolField(TEXT("is_read_only"), IsReadOnly);
		TextBox->SetIsReadOnly(IsReadOnly);
		
		bool IsPassword = false;
		KwargsObjectRef->TryGetBoolField(TEXT("is_password"), IsPassword);
		TextBox->SetIsPassword(IsPassword);
	}
	// ListView
	else if (ComponentType.Equals(TEXT("ListView"), ESearchCase::IgnoreCase))
	{
		UListView* ListView = WidgetBlueprint->WidgetTree->ConstructWidget<UListView>(UListView::StaticClass(), *ComponentName);
		CreatedWidget = ListView;
		
		// ListView requires more setup in Blueprint for data binding to be useful
		// We'll create the base widget here, and the user can configure it in the Blueprint editor
		
		// Set basic properties if available
		FString SelectionMode;
		if (KwargsObjectRef->TryGetStringField(TEXT("selection_mode"), SelectionMode))
		{
			if (SelectionMode.Equals(TEXT("Single"), ESearchCase::IgnoreCase))
				ListView->SetSelectionMode(ESelectionMode::Single);
			else if (SelectionMode.Equals(TEXT("Multi"), ESearchCase::IgnoreCase))
				ListView->SetSelectionMode(ESelectionMode::Multi);
			else if (SelectionMode.Equals(TEXT("None"), ESearchCase::IgnoreCase))
				ListView->SetSelectionMode(ESelectionMode::None);
		}
	}
	// TileView
	else if (ComponentType.Equals(TEXT("TileView"), ESearchCase::IgnoreCase))
	{
		UTileView* TileView = WidgetBlueprint->WidgetTree->ConstructWidget<UTileView>(UTileView::StaticClass(), *ComponentName);
		CreatedWidget = TileView;
		
		// TileView requires more setup in Blueprint for data binding to be useful
		// We'll create the base widget here, and the user can configure it in the Blueprint editor
		
		// Set basic properties if available
		float EntryWidth = 128.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("entry_width"), EntryWidth);
		TileView->SetEntryWidth(EntryWidth);
		
		float EntryHeight = 128.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("entry_height"), EntryHeight);
		TileView->SetEntryHeight(EntryHeight);
	}
	// TreeView
	else if (ComponentType.Equals(TEXT("TreeView"), ESearchCase::IgnoreCase))
	{
		UTreeView* TreeView = WidgetBlueprint->WidgetTree->ConstructWidget<UTreeView>(UTreeView::StaticClass(), *ComponentName);
		CreatedWidget = TreeView;
		
		// TreeView requires more setup in Blueprint for data binding to be useful
		// We'll create the base widget here, and the user can configure it in the Blueprint editor
	}
	// SafeZone
	else if (ComponentType.Equals(TEXT("SafeZone"), ESearchCase::IgnoreCase))
	{
		USafeZone* SafeZone = WidgetBlueprint->WidgetTree->ConstructWidget<USafeZone>(USafeZone::StaticClass(), *ComponentName);
		CreatedWidget = SafeZone;
		
		// Apply safe zone specific properties
		bool IsTitleSafe = true;
		KwargsObjectRef->TryGetBoolField(TEXT("is_title_safe"), IsTitleSafe);
		// UE 5.5 doesn't have SetIsTitleSafe method for USafeZone
		// SafeZone->SetIsTitleSafe(IsTitleSafe);
		
		// Set padding scale if provided
		float PaddingScale = 1.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("padding_scale"), PaddingScale);
		// UE 5.5 doesn't have SetPadding method for USafeZone
		// SafeZone->SetPadding(FMargin(PaddingScale));
	}
	// MenuAnchor
	else if (ComponentType.Equals(TEXT("MenuAnchor"), ESearchCase::IgnoreCase))
	{
		UMenuAnchor* MenuAnchor = WidgetBlueprint->WidgetTree->ConstructWidget<UMenuAnchor>(UMenuAnchor::StaticClass(), *ComponentName);
		CreatedWidget = MenuAnchor;
		
		// MenuAnchor requires setup in Blueprint to be useful
		// For the menu content, we'll need to bind a function in Blueprint
		
		// Set basic properties if available
		FString Placement;
		if (KwargsObjectRef->TryGetStringField(TEXT("placement"), Placement))
		{
			if (Placement.Equals(TEXT("ComboBox"), ESearchCase::IgnoreCase))
				MenuAnchor->SetPlacement(MenuPlacement_ComboBox);
			else if (Placement.Equals(TEXT("BelowAnchor"), ESearchCase::IgnoreCase))
				MenuAnchor->SetPlacement(MenuPlacement_BelowAnchor);
			else if (Placement.Equals(TEXT("CenteredBelowAnchor"), ESearchCase::IgnoreCase))
				MenuAnchor->SetPlacement(MenuPlacement_CenteredBelowAnchor);
			else if (Placement.Equals(TEXT("AboveAnchor"), ESearchCase::IgnoreCase))
				MenuAnchor->SetPlacement(MenuPlacement_AboveAnchor);
			else if (Placement.Equals(TEXT("CenteredAboveAnchor"), ESearchCase::IgnoreCase))
				MenuAnchor->SetPlacement(MenuPlacement_CenteredAboveAnchor);
		}
	}
	// NativeWidgetHost
	else if (ComponentType.Equals(TEXT("NativeWidgetHost"), ESearchCase::IgnoreCase))
	{
		UNativeWidgetHost* NativeWidgetHost = WidgetBlueprint->WidgetTree->ConstructWidget<UNativeWidgetHost>(UNativeWidgetHost::StaticClass(), *ComponentName);
		CreatedWidget = NativeWidgetHost;
		// NativeWidgetHost requires platform-specific setup to be useful
		UE_LOG(LogTemp, Warning, TEXT("Created NativeWidgetHost '%s'. Additional platform-specific setup may be required in Blueprint."), *ComponentName);
	}
	// BackgroundBlur
	else if (ComponentType.Equals(TEXT("BackgroundBlur"), ESearchCase::IgnoreCase))
	{
		UBackgroundBlur* BackgroundBlur = WidgetBlueprint->WidgetTree->ConstructWidget<UBackgroundBlur>(UBackgroundBlur::StaticClass(), *ComponentName);
		CreatedWidget = BackgroundBlur;
		
		// Apply background blur specific properties
		float BlurStrength = 5.0f;
		KwargsObjectRef->TryGetNumberField(TEXT("blur_strength"), BlurStrength);
		BackgroundBlur->SetBlurStrength(BlurStrength);
		
		bool ApplyAlphaToBlur = true;
		KwargsObjectRef->TryGetBoolField(TEXT("apply_alpha_to_blur"), ApplyAlphaToBlur);
		BackgroundBlur->SetApplyAlphaToBlur(ApplyAlphaToBlur);
		
		// Set blur background color if provided
		TArray<TSharedPtr<FJsonValue>> ColorArray;
		if (GetJsonArray(KwargsObjectRef, TEXT("background_color"), ColorArray) && ColorArray.Num() >= 3)
		{
			float R = ColorArray[0]->AsNumber();
			float G = ColorArray[1]->AsNumber();
			float B = ColorArray[2]->AsNumber();
			float A = ColorArray.Num() >= 4 ? ColorArray[3]->AsNumber() : 1.0f;
			
			// UE 5.5 doesn't have SetBackgroundColor method for UBackgroundBlur
			// BackgroundBlur->SetBackgroundColor(FLinearColor(R, G, B, A));
		}
	}
	// StackBox
	else if (ComponentType.Equals(TEXT("StackBox"), ESearchCase::IgnoreCase))
	{
		// UStackBox doesn't exist in standard UE5, we'll use UVerticalBox as a fallback
		UVerticalBox* StackBox = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *ComponentName);
		CreatedWidget = StackBox;
		
		// Log a warning that we're using VerticalBox instead
		UE_LOG(LogTemp, Warning, TEXT("StackBox is not available in this UE version. Using VerticalBox instead for '%s'."), *ComponentName);
	}
	// UniformGridPanel
	else if (ComponentType.Equals(TEXT("UniformGridPanel"), ESearchCase::IgnoreCase))
	{
		UUniformGridPanel* UniformGrid = WidgetBlueprint->WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), *ComponentName);
		CreatedWidget = UniformGrid;
		
		// Apply uniform grid panel specific properties
		int32 SlotPadding = 0;
		KwargsObjectRef->TryGetNumberField(TEXT("slot_padding"), SlotPadding);
		UniformGrid->SetSlotPadding(FVector2D(SlotPadding, SlotPadding));
		
		int32 MinDesiredSlotWidth = 0;
		KwargsObjectRef->TryGetNumberField(TEXT("min_desired_slot_width"), MinDesiredSlotWidth);
		UniformGrid->SetMinDesiredSlotWidth(MinDesiredSlotWidth);
		
		int32 MinDesiredSlotHeight = 0;
		KwargsObjectRef->TryGetNumberField(TEXT("min_desired_slot_height"), MinDesiredSlotHeight);
		UniformGrid->SetMinDesiredSlotHeight(MinDesiredSlotHeight);
	}
	// Default/Unknown case
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported component type: %s"), *ComponentType));
	}

	// Make sure widget was created
	if (!CreatedWidget)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create widget component"));
	}

	// Add to canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root Canvas Panel not found"));
	}

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(CreatedWidget);
	PanelSlot->SetPosition(Position);
	PanelSlot->SetSize(Size);

	// Mark the package dirty and compile
	WidgetBlueprint->MarkPackageDirty();
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WidgetBlueprint);

	// Build success response
	Response->SetStringField(TEXT("status"), TEXT("success"));
	Response->SetStringField(TEXT("component_name"), ComponentName);
	Response->SetStringField(TEXT("component_type"), ComponentType);
	
	TArray<TSharedPtr<FJsonValue>> PosArray;
	PosArray.Add(MakeShared<FJsonValueNumber>(Position.X));
	PosArray.Add(MakeShared<FJsonValueNumber>(Position.Y));
	Response->SetArrayField(TEXT("position"), PosArray);
	
	TArray<TSharedPtr<FJsonValue>> SizeArray;
	SizeArray.Add(MakeShared<FJsonValueNumber>(Size.X));
	SizeArray.Add(MakeShared<FJsonValueNumber>(Size.Y));
	Response->SetArrayField(TEXT("size"), SizeArray);

	return Response;
}