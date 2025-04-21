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

	return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown UMG command: %s"), *CommandName));
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