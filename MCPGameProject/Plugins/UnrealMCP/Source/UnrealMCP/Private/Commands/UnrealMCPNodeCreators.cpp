#include "Commands/UnrealMCPNodeCreators.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"

// Blueprint Action Database includes
#include "K2Node.h"
#include "BlueprintActionDatabase.h"
#include "BlueprintActionFilter.h"
#include "BlueprintActionMenuBuilder.h"
#include "BlueprintActionMenuItem.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_BreakStruct.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_MapForEach.h"
#include "K2Node_SetForEach.h"
#include "K2Node_InputAction.h"
#include "K2Node_Self.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"

// Phase 2 includes removed - using universal dynamic creation via Blueprint Action Database
#include "Engine/UserDefinedStruct.h"
#include "Engine/UserDefinedEnum.h"
#include "KismetCompiler.h"
#include "BlueprintNodeSpawner.h"
#include "Kismet/KismetMathLibrary.h"

// For pin type analysis
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"

// Additional includes for node creation
#include "Commands/UnrealMCPCommonUtils.h"
#include "Commands/UnrealMCPNodeCreationHelpers.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

FString UnrealMCPNodeCreators::CreateNodeByActionName(const FString& BlueprintName, const FString& FunctionName, const FString& ClassName, const FString& NodePosition, const FString& JsonParams)
{
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    
    // Parse JSON parameters using helper
    TSharedPtr<FJsonObject> ParamsObject;
    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: JsonParams = '%s'"), *JsonParams);
    if (!UnrealMCPNodeCreationHelpers::ParseJsonParameters(JsonParams, ParamsObject, ResultObj))
    {
        return UnrealMCPNodeCreationHelpers::BuildNodeResult(false, TEXT("Invalid JSON parameters"));
    }
    
    // Find the blueprint
    UBlueprint* Blueprint = nullptr;
    
    // Find blueprint by searching for it in the asset registry
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> BlueprintAssets;
    AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), BlueprintAssets);
    
    for (const FAssetData& AssetData : BlueprintAssets)
    {
        FString AssetName = AssetData.AssetName.ToString();
        if (AssetName.Contains(BlueprintName) || BlueprintName.Contains(AssetName))
        {
            Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
            if (Blueprint)
                break;
        }
    }
    
    if (!Blueprint)
    {
        return UnrealMCPNodeCreationHelpers::BuildNodeResult(false, FString::Printf(TEXT("Blueprint '%s' not found"), *BlueprintName));
    }
    
    // Get the event graph
    UEdGraph* EventGraph = nullptr;
    for (UEdGraph* Graph : Blueprint->UbergraphPages)
    {
        if (Graph && Graph->GetFName() == "EventGraph")
        {
            EventGraph = Graph;
            break;
        }
    }
    
    if (!EventGraph)
    {
        return UnrealMCPNodeCreationHelpers::BuildNodeResult(false, TEXT("Could not find EventGraph in blueprint"));
    }
    
    // Parse node position using helper
    int32 PositionX, PositionY;
    UnrealMCPNodeCreationHelpers::ParseNodePosition(NodePosition, PositionX, PositionY);
    
    UEdGraphNode* NewNode = nullptr;
    FString NodeTitle = TEXT("Unknown");
    FString NodeType = TEXT("Unknown");
    UClass* TargetClass = nullptr;
    
    // Check if this is a control flow node request
    if (FunctionName.Equals(TEXT("Branch"), ESearchCase::IgnoreCase) || 
        FunctionName.Equals(TEXT("IfThenElse"), ESearchCase::IgnoreCase) ||
        FunctionName.Equals(TEXT("UK2Node_IfThenElse"), ESearchCase::IgnoreCase))
    {
        UK2Node_IfThenElse* BranchNode = NewObject<UK2Node_IfThenElse>(EventGraph);
        BranchNode->NodePosX = PositionX;
        BranchNode->NodePosY = PositionY;
        BranchNode->CreateNewGuid();
        EventGraph->AddNode(BranchNode, true, true);
        BranchNode->PostPlacedNewNode();
        BranchNode->AllocateDefaultPins();
        NewNode = BranchNode;
        NodeTitle = TEXT("Branch");
        NodeType = TEXT("UK2Node_IfThenElse");
    }
    else if (FunctionName.Equals(TEXT("Sequence"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("ExecutionSequence"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("UK2Node_ExecutionSequence"), ESearchCase::IgnoreCase))
    {
        UK2Node_ExecutionSequence* SequenceNode = NewObject<UK2Node_ExecutionSequence>(EventGraph);
        SequenceNode->NodePosX = PositionX;
        SequenceNode->NodePosY = PositionY;
        SequenceNode->CreateNewGuid();
        EventGraph->AddNode(SequenceNode, true, true);
        SequenceNode->PostPlacedNewNode();
        SequenceNode->AllocateDefaultPins();
        NewNode = SequenceNode;
        NodeTitle = TEXT("Sequence");
        NodeType = TEXT("UK2Node_ExecutionSequence");
    }

    else if (FunctionName.Equals(TEXT("CustomEvent"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Custom Event"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("UK2Node_CustomEvent"), ESearchCase::IgnoreCase))
    {
        UK2Node_CustomEvent* CustomEventNode = NewObject<UK2Node_CustomEvent>(EventGraph);
        
        // Set custom event name from parameters if provided
        FString EventName = TEXT("CustomEvent"); // Default name
        if (ParamsObject.IsValid())
        {
            FString ParamEventName;
            if (ParamsObject->TryGetStringField(TEXT("event_name"), ParamEventName) && !ParamEventName.IsEmpty())
            {
                EventName = ParamEventName;
            }
        }
        
        CustomEventNode->CustomFunctionName = FName(*EventName);
        CustomEventNode->NodePosX = PositionX;
        CustomEventNode->NodePosY = PositionY;
        CustomEventNode->CreateNewGuid();
        EventGraph->AddNode(CustomEventNode, true, true);
        CustomEventNode->PostPlacedNewNode();
        CustomEventNode->AllocateDefaultPins();
        NewNode = CustomEventNode;
        NodeTitle = EventName;
        NodeType = TEXT("UK2Node_CustomEvent");
    }
    else if (FunctionName.Equals(TEXT("Cast"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("DynamicCast"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("UK2Node_DynamicCast"), ESearchCase::IgnoreCase))
    {
        UK2Node_DynamicCast* CastNode = NewObject<UK2Node_DynamicCast>(EventGraph);
        
        // Set target type if provided in parameters
        if (ParamsObject.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: ParamsObject is valid for Cast node"));
            FString TargetTypeName;
            
            // Check if target_type is in kwargs sub-object
            const TSharedPtr<FJsonObject>* KwargsObject;
            if (ParamsObject->TryGetObjectField(TEXT("kwargs"), KwargsObject) && KwargsObject->IsValid())
            {
                UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found kwargs object"));
                if ((*KwargsObject)->TryGetStringField(TEXT("target_type"), TargetTypeName) && !TargetTypeName.IsEmpty())
                {
                    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found target_type in kwargs: '%s'"), *TargetTypeName);
                }
            }
            // Also check at root level for backwards compatibility
            else if (ParamsObject->TryGetStringField(TEXT("target_type"), TargetTypeName) && !TargetTypeName.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found target_type parameter: '%s'"), *TargetTypeName);
            }
            
            if (!TargetTypeName.IsEmpty())
            {
                // Find the class to cast to
                UClass* CastTargetClass = nullptr;
                
                UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Looking for target type '%s'"), *TargetTypeName);
                
                // Common class mappings
                if (TargetTypeName.Equals(TEXT("PlayerController"), ESearchCase::IgnoreCase))
                {
                    CastTargetClass = APlayerController::StaticClass();
                    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found PlayerController class"));
                }
                else if (TargetTypeName.Equals(TEXT("Pawn"), ESearchCase::IgnoreCase))
                {
                    CastTargetClass = APawn::StaticClass();
                    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found Pawn class"));
                }
                else if (TargetTypeName.Equals(TEXT("Actor"), ESearchCase::IgnoreCase))
                {
                    CastTargetClass = AActor::StaticClass();
                    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found Actor class"));
                }
                else
                {
                    // Try to find the class by name
                    CastTargetClass = UClass::TryFindTypeSlow<UClass>(TargetTypeName);
                    if (!CastTargetClass)
                    {
                        // Try with /Script/Engine. prefix
                        FString EnginePath = FString::Printf(TEXT("/Script/Engine.%s"), *TargetTypeName);
                        CastTargetClass = LoadClass<UObject>(nullptr, *EnginePath);
                    }
                    
                    // If still not found, try to find it as a Blueprint class
                    if (!CastTargetClass)
                    {
                        // Reuse existing AssetRegistryModule and BlueprintAssets variables
                        FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
                        TArray<FAssetData> BPAssets;
                        AssetReg.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), BPAssets);
                        
                        for (const FAssetData& AssetData : BPAssets)
                        {
                            FString AssetName = AssetData.AssetName.ToString();
                            if (AssetName.Contains(TargetTypeName) || TargetTypeName.Contains(AssetName))
                            {
                                if (UBlueprint* TargetBP = Cast<UBlueprint>(AssetData.GetAsset()))
                                {
                                    CastTargetClass = TargetBP->GeneratedClass;
                                    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found Blueprint class '%s'"), *CastTargetClass->GetName());
                                    break;
                                }
                            }
                        }
                    }
                }
                
                if (CastTargetClass)
                {
                    CastNode->TargetType = CastTargetClass;
                    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Set cast target type to '%s'"), *CastTargetClass->GetName());
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("CreateNodeByActionName: Could not find target type '%s'"), *TargetTypeName);
                }
            }
        }
        
        CastNode->NodePosX = PositionX;
        CastNode->NodePosY = PositionY;
        CastNode->CreateNewGuid();
        EventGraph->AddNode(CastNode, true, true);
        CastNode->PostPlacedNewNode();
        CastNode->AllocateDefaultPins();
        NewNode = CastNode;
        NodeTitle = TEXT("Cast");
        NodeType = TEXT("UK2Node_DynamicCast");
    }
    // Handle loop macros in Blueprint Action Database
    else if (FunctionName.Equals(TEXT("Loop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("For Loop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("ForLoop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Loop with Break"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("LoopWithBreak"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("For Loop with Break"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("ForLoopWithBreak"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("For Each Loop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("ForEachLoop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("While Loop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("WhileLoop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Do Once"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("DoOnce"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Do N"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("DoN"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("MultiGate"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Multi Gate"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Flip Flop"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("FlipFlop"), ESearchCase::IgnoreCase))
    {
        // Use Blueprint Action Database to find the macro
        FBlueprintActionDatabase& ActionDatabase = FBlueprintActionDatabase::Get();
        
        // Find the macro blueprint
        UBlueprint* MacroBlueprint = nullptr;
        
        // Search in engine macros
        TArray<FAssetData> MacroBlueprintAssets;
        FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        AssetReg.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), MacroBlueprintAssets);
        
        for (const FAssetData& AssetData : MacroBlueprintAssets)
        {
            FString AssetName = AssetData.AssetName.ToString();
            FString PackagePath = AssetData.PackageName.ToString();
            
            UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Checking asset '%s' in package '%s'"), *AssetName, *PackagePath);
            
            // Check if this is a macro blueprint
            if (PackagePath.Contains(TEXT("Engine/EditorBlueprintResources")) || PackagePath.Contains(TEXT("Engine/EngineMacros")))
            {
                // Check if the asset name matches our function name
                if (AssetName.Contains(FunctionName) || FunctionName.Contains(AssetName))
                {
                    MacroBlueprint = Cast<UBlueprint>(AssetData.GetAsset());
                    if (MacroBlueprint)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found macro blueprint '%s'"), *MacroBlueprint->GetName());
                        break;
                    }
                }
            }
        }
        
        if (MacroBlueprint)
            {
                // Get the macro's function graph
                UEdGraph* MacroGraph = nullptr;
                for (UEdGraph* Graph : MacroBlueprint->MacroGraphs)
                {
                    if (Graph)
                    {
                        MacroGraph = Graph;
                        break;
                    }
                }
                
                if (MacroGraph)
                {
                    // Note: Blueprint Action Database API changed in UE 5.6 - simplified approach
                    UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found macro blueprint, attempting direct macro creation"));
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("CreateNodeByActionName: No macro graph found"));
                }
            }
            
            // If we didn't find a macro, try to create the node manually
            if (!NewNode)
            {
                UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: No macro found, trying to create manually"));
                
                // For regular "For Loop" try to find the engine macro
                if (FunctionName.Equals(TEXT("Loop"), ESearchCase::IgnoreCase) ||
                    FunctionName.Equals(TEXT("For Loop"), ESearchCase::IgnoreCase) ||
                    FunctionName.Equals(TEXT("ForLoop"), ESearchCase::IgnoreCase))
                {
                    // Try to find the ForLoop macro blueprint
                    UBlueprint* ForLoopMacro = LoadObject<UBlueprint>(nullptr, TEXT("/Engine/EditorBlueprintResources/StandardMacros/ForLoop.ForLoop"));
                    if (ForLoopMacro)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Found ForLoop macro blueprint"));
                        
                        // Create macro instance
                        UK2Node_MacroInstance* MacroInstance = NewObject<UK2Node_MacroInstance>(EventGraph);
                        MacroInstance->SetMacroGraph(ForLoopMacro->MacroGraphs[0]);
                        MacroInstance->NodePosX = PositionX;
                        MacroInstance->NodePosY = PositionY;
                        MacroInstance->CreateNewGuid();
                        EventGraph->AddNode(MacroInstance, true, true);
                        MacroInstance->PostPlacedNewNode();
                        MacroInstance->AllocateDefaultPins();
                        NewNode = MacroInstance;
                        NodeTitle = TEXT("For Loop");
                        NodeType = TEXT("UK2Node_MacroInstance");
                    }
                }
            }
            
            if (!NewNode)
            {
                // Note: Blueprint Action Database API changed in UE 5.6 - using simplified approach
                UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: No macro found, skipping advanced action database lookup"));
                
                ResultObj->SetBoolField(TEXT("success"), false);
                ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Could not find engine macro blueprint for '%s'. This macro may not be available in the current Unreal Engine version."), *FunctionName));
                
                FString OutputString;
                TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
                FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
                return OutputString;
            }
    }

    // Variable getter/setter node creation - check this FIRST before function lookup
    else if (FunctionName.StartsWith(TEXT("Get ")) || FunctionName.StartsWith(TEXT("Set ")) ||
        FunctionName.Equals(TEXT("UK2Node_VariableGet"), ESearchCase::IgnoreCase) ||
        FunctionName.Equals(TEXT("UK2Node_VariableSet"), ESearchCase::IgnoreCase))
    {
        FString VarName = FunctionName;
        bool bIsGetter = false;
        if (VarName.StartsWith(TEXT("Get ")))
        {
            VarName = VarName.RightChop(4);
            bIsGetter = true;
        }
        else if (VarName.StartsWith(TEXT("Set ")))
        {
            VarName = VarName.RightChop(4);
        }
        // Try to find the variable in the Blueprint
        bool bFound = false;
        for (const FBPVariableDescription& VarDesc : Blueprint->NewVariables)
        {
            if (VarDesc.VarName.ToString().Equals(VarName, ESearchCase::IgnoreCase))
            {
                if (bIsGetter)
                {
                    UK2Node_VariableGet* GetterNode = NewObject<UK2Node_VariableGet>(EventGraph);
                    GetterNode->VariableReference.SetSelfMember(*VarName);
                    GetterNode->NodePosX = PositionX;
                    GetterNode->NodePosY = PositionY;
                    GetterNode->CreateNewGuid();
                    EventGraph->AddNode(GetterNode, true, true);
                    GetterNode->PostPlacedNewNode();
                    GetterNode->AllocateDefaultPins();
                    NewNode = GetterNode;
                    NodeTitle = FString::Printf(TEXT("Get %s"), *VarName);
                    NodeType = TEXT("UK2Node_VariableGet");
                }
                else
                {
                    UK2Node_VariableSet* SetterNode = NewObject<UK2Node_VariableSet>(EventGraph);
                    SetterNode->VariableReference.SetSelfMember(*VarName);
                    SetterNode->NodePosX = PositionX;
                    SetterNode->NodePosY = PositionY;
                    SetterNode->CreateNewGuid();
                    EventGraph->AddNode(SetterNode, true, true);
                    SetterNode->PostPlacedNewNode();
                    SetterNode->AllocateDefaultPins();
                    NewNode = SetterNode;
                    NodeTitle = FString::Printf(TEXT("Set %s"), *VarName);
                    NodeType = TEXT("UK2Node_VariableSet");
                }
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            ResultObj->SetBoolField(TEXT("success"), false);
            ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Variable '%s' not found in Blueprint '%s'"), *VarName, *BlueprintName));
            FString OutputString;
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
            FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
            return OutputString;
        }
    }

    // Special loop node types that are not macros  
    else if (FunctionName.Equals(TEXT("For Each Loop (Map)"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Map ForEach"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("K2Node_MapForEach"), ESearchCase::IgnoreCase))
    {
        UK2Node_MapForEach* MapForEachNode = NewObject<UK2Node_MapForEach>(EventGraph);
        MapForEachNode->NodePosX = PositionX;
        MapForEachNode->NodePosY = PositionY;
        MapForEachNode->CreateNewGuid();
        EventGraph->AddNode(MapForEachNode, true, true);
        MapForEachNode->PostPlacedNewNode();
        MapForEachNode->AllocateDefaultPins();
        NewNode = MapForEachNode;
        NodeTitle = TEXT("For Each Loop (Map)");
        NodeType = TEXT("UK2Node_MapForEach");
    }
    else if (FunctionName.Equals(TEXT("For Each Loop (Set)"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("Set ForEach"), ESearchCase::IgnoreCase) ||
             FunctionName.Equals(TEXT("K2Node_SetForEach"), ESearchCase::IgnoreCase))
    {
        UK2Node_SetForEach* SetForEachNode = NewObject<UK2Node_SetForEach>(EventGraph);
        SetForEachNode->NodePosX = PositionX;
        SetForEachNode->NodePosY = PositionY;
        SetForEachNode->CreateNewGuid();
        EventGraph->AddNode(SetForEachNode, true, true);
        SetForEachNode->PostPlacedNewNode();
        SetForEachNode->AllocateDefaultPins();
        NewNode = SetForEachNode;
        NodeTitle = TEXT("For Each Loop (Set)");
        NodeType = TEXT("UK2Node_SetForEach");
    }
    
    
    // ===== UNIVERSAL DYNAMIC NODE CREATION USING BLUEPRINT ACTION DATABASE =====
    // This replaces hardcoded node creation with Unreal's native spawner system
    else if (UnrealMCPNodeCreationHelpers::TryCreateNodeUsingBlueprintActionDatabase(FunctionName, EventGraph, PositionX, PositionY, NewNode, NodeTitle, NodeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Successfully created node '%s' using Blueprint Action Database"), *NodeTitle);
    }
    else
    {
        // Try to find the function and create a function call node
        UFunction* TargetFunction = nullptr;
        TargetClass = nullptr;
        
        // Find target class using helper
        TargetClass = UnrealMCPNodeCreationHelpers::FindTargetClass(ClassName);
        if (TargetClass)
        {
            TargetFunction = TargetClass->FindFunctionByName(*FunctionName);
        }
        else
        {
            // Try to find the function in common math/utility classes
            TArray<UClass*> CommonClasses = {
                UKismetMathLibrary::StaticClass(),
                UKismetSystemLibrary::StaticClass(),
                UGameplayStatics::StaticClass()
            };
            
            for (UClass* TestClass : CommonClasses)
            {
                TargetFunction = TestClass->FindFunctionByName(*FunctionName);
                if (TargetFunction)
                {
                    TargetClass = TestClass;
                    break;
                }
            }
        }
        
        if (!TargetFunction)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateNodeByActionName: Function '%s' not found"), *FunctionName);
            return UnrealMCPNodeCreationHelpers::BuildNodeResult(false, FString::Printf(TEXT("Function '%s' not found and not a recognized control flow node"), *FunctionName));
        }
        
        UE_LOG(LogTemp, Log, TEXT("CreateNodeByActionName: Found function '%s' in class '%s'"), *FunctionName, TargetClass ? *TargetClass->GetName() : TEXT("Unknown"));
        
        // Create the function call node
        UK2Node_CallFunction* FunctionNode = NewObject<UK2Node_CallFunction>(EventGraph);
        FunctionNode->FunctionReference.SetExternalMember(TargetFunction->GetFName(), TargetClass);
        FunctionNode->NodePosX = PositionX;
        FunctionNode->NodePosY = PositionY;
        FunctionNode->CreateNewGuid();
        EventGraph->AddNode(FunctionNode, true, true);
        FunctionNode->PostPlacedNewNode();
        FunctionNode->AllocateDefaultPins();
        NewNode = FunctionNode;
        NodeTitle = FunctionName;
        NodeType = TEXT("UK2Node_CallFunction");
    }
    
    if (!NewNode)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateNodeByActionName: Failed to create node for '%s'"), *FunctionName);
        return UnrealMCPNodeCreationHelpers::BuildNodeResult(false, FString::Printf(TEXT("Failed to create node for '%s'"), *FunctionName));
    }
    
    UE_LOG(LogTemp, Log, TEXT("CreateNodeByActionName: Successfully created node '%s' of type '%s'"), *NodeTitle, *NodeType);
    
    // Mark blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    
    // Return success result using helper
    return UnrealMCPNodeCreationHelpers::BuildNodeResult(true, FString::Printf(TEXT("Successfully created '%s' node (%s)"), *NodeTitle, *NodeType),
                          BlueprintName, FunctionName, NewNode, NodeTitle, NodeType, TargetClass, PositionX, PositionY);
} 