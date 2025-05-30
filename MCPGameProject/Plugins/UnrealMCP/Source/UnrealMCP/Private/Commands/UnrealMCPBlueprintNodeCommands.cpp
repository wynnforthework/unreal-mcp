#include "Commands/UnrealMCPBlueprintNodeCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/DataTable.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_InputAction.h"
#include "K2Node_Self.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "GameFramework/InputSettings.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_MacroInstance.h"
#include "Misc/PackageName.h"
#include "K2Node_BreakStruct.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EditorAssetLibrary.h"
#include "EnhancedInput/Public/InputAction.h"
#include "EnhancedInput/Public/InputMappingContext.h"

// No longer needed as we're using LogTemp
// DEFINE_LOG_CATEGORY_STATIC(LogUnrealMCP, Log, All);

FUnrealMCPBlueprintNodeCommands::FUnrealMCPBlueprintNodeCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("connect_blueprint_nodes"))
    {
        return HandleConnectBlueprintNodes(Params);
    }
    else if (CommandType == TEXT("add_blueprint_get_self_component_reference"))
    {
        return HandleAddBlueprintGetSelfComponentReference(Params);
    }
    else if (CommandType == TEXT("add_blueprint_event_node"))
    {
        return HandleAddBlueprintEvent(Params);
    }
    else if (CommandType == TEXT("add_blueprint_function_node"))
    {
        return HandleAddBlueprintFunctionCall(Params);
    }
    else if (CommandType == TEXT("add_blueprint_variable"))
    {
        return HandleAddBlueprintVariable(Params);
    }
    else if (CommandType == TEXT("add_blueprint_input_action_node"))
    {
        return HandleAddBlueprintInputActionNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_self_reference"))
    {
        return HandleAddBlueprintSelfReference(Params);
    }
    else if (CommandType == TEXT("find_blueprint_nodes"))
    {
        return HandleFindBlueprintNodes(Params);
    }
    else if (CommandType == TEXT("add_blueprint_custom_event_node"))
    {
        return HandleAddBlueprintCustomEventNode(Params);
    }
    else if (CommandType == TEXT("get_variable_info"))
    {
        return HandleGetVariableInfo(Params);
    }
    else if (CommandType == TEXT("add_enhanced_input_action_node"))
    {
        return HandleAddEnhancedInputActionNode(Params);
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown blueprint node command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleConnectBlueprintNodes(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString SourceNodeId;
    if (!Params->TryGetStringField(TEXT("source_node_id"), SourceNodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_node_id' parameter"));
    }

    FString TargetNodeId;
    if (!Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'target_node_id' parameter"));
    }

    FString SourcePinName;
    if (!Params->TryGetStringField(TEXT("source_pin"), SourcePinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_pin' parameter"));
    }

    FString TargetPinName;
    if (!Params->TryGetStringField(TEXT("target_pin"), TargetPinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'target_pin' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Find the nodes
    UEdGraphNode* SourceNode = nullptr;
    UEdGraphNode* TargetNode = nullptr;
    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        if (Node->NodeGuid.ToString() == SourceNodeId)
        {
            SourceNode = Node;
        }
        else if (Node->NodeGuid.ToString() == TargetNodeId)
        {
            TargetNode = Node;
        }
    }

    if (!SourceNode || !TargetNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Source or target node not found"));
    }

    // Connect the nodes
    if (FUnrealMCPCommonUtils::ConnectGraphNodes(EventGraph, SourceNode, SourcePinName, TargetNode, TargetPinName))
    {
        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("source_node_id"), SourceNodeId);
        ResultObj->SetStringField(TEXT("target_node_id"), TargetNodeId);
        return ResultObj;
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to connect nodes"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintGetSelfComponentReference(const TSharedPtr<FJsonObject>& Params)
{
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

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }
    
    // We'll skip component verification since the GetAllNodes API may have changed in UE5.5
    
    // Create the variable get node directly
    UK2Node_VariableGet* GetComponentNode = NewObject<UK2Node_VariableGet>(EventGraph);
    if (!GetComponentNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create get component node"));
    }
    
    // Set up the variable reference properly for UE5.5
    FMemberReference& VarRef = GetComponentNode->VariableReference;
    VarRef.SetSelfMember(FName(*ComponentName));
    
    // Set node position
    GetComponentNode->NodePosX = NodePosition.X;
    GetComponentNode->NodePosY = NodePosition.Y;
    
    // Add to graph
    EventGraph->AddNode(GetComponentNode);
    GetComponentNode->CreateNewGuid();
    GetComponentNode->PostPlacedNewNode();
    GetComponentNode->AllocateDefaultPins();
    
    // Explicitly reconstruct node for UE5.5
    GetComponentNode->ReconstructNode();
    
    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), GetComponentNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintEvent(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString EventName;
    if (!Params->TryGetStringField(TEXT("event_name"), EventName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'event_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the event node
    UK2Node_Event* EventNode = FUnrealMCPCommonUtils::CreateEventNode(EventGraph, EventName, NodePosition);
    if (!EventNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create event node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
    return ResultObj;
}

// Helper function to create special Blueprint nodes by name
static UEdGraphNode* CreateSpecialNodeByName(const FString& NodeType, UEdGraph* Graph) {
    if (NodeType.Equals(TEXT("Branch"), ESearchCase::IgnoreCase)) {
        return NewObject<UK2Node_IfThenElse>(Graph);
    }
    if (NodeType.Equals(TEXT("Sequence"), ESearchCase::IgnoreCase)) {
        return NewObject<UK2Node_ExecutionSequence>(Graph);
    }
    if (NodeType.Equals(TEXT("Print String"), ESearchCase::IgnoreCase)) {
        UK2Node_CallFunction* PrintStringNode = NewObject<UK2Node_CallFunction>(Graph);
        PrintStringNode->FunctionReference.SetExternalMember(
            FName(TEXT("PrintString")),
            UKismetSystemLibrary::StaticClass()
        );
        return PrintStringNode;
    }
    auto CreateMacroInstance = [&](const FString& MacroName) -> UK2Node_MacroInstance* {
        const FString MacroAssetPath = TEXT("/Engine/EditorBlueprintResources/StandardMacros.StandardMacros");
        UBlueprint* MacroBP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *MacroAssetPath));
        if (!MacroBP) {
            UE_LOG(LogTemp, Error, TEXT("Failed to load StandardMacros asset at %s"), *MacroAssetPath);
            return nullptr;
        }
        // Log all macro graph names for debugging (use MacroGraphs, not UbergraphPages)
        FString AllMacroNames;
        for (UEdGraph* MacroGraph : MacroBP->MacroGraphs) {
            if (MacroGraph) {
                AllMacroNames += MacroGraph->GetName() + TEXT(", ");
            }
        }
        UE_LOG(LogTemp, Display, TEXT("Available macro graphs in StandardMacros: %s"), *AllMacroNames);
        // Find the macro graph by exact name
        for (UEdGraph* MacroGraph : MacroBP->MacroGraphs) {
            if (MacroGraph && MacroGraph->GetName().Equals(MacroName, ESearchCase::CaseSensitive)) {
                UK2Node_MacroInstance* MacroNode = NewObject<UK2Node_MacroInstance>(Graph);
                MacroNode->SetMacroGraph(MacroGraph);
                return MacroNode;
            }
        }
        UE_LOG(LogTemp, Error, TEXT("Macro graph '%s' not found in StandardMacros asset!"), *MacroName);
        return nullptr;
    };
    if (NodeType.Equals(TEXT("ForEachLoop"), ESearchCase::IgnoreCase)) {
        return CreateMacroInstance(TEXT("ForEachLoop"));
    }
    if (NodeType.Equals(TEXT("ForLoop"), ESearchCase::IgnoreCase)) {
        return CreateMacroInstance(TEXT("ForLoop"));
    }
    return nullptr;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintFunctionCall(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString FunctionName;
    if (!Params->TryGetStringField(TEXT("function_name"), FunctionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'function_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Check for target parameter (optional)
    FString Target;
    Params->TryGetStringField(TEXT("target"), Target);

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Generic special node creation
    UEdGraphNode* SpecialNode = CreateSpecialNodeByName(FunctionName, EventGraph);
    if (SpecialNode)
    {
        SpecialNode->NodePosX = NodePosition.X;
        SpecialNode->NodePosY = NodePosition.Y;
        // If this is a comment node, set text and size if provided
        
        EventGraph->AddNode(SpecialNode, true);
        SpecialNode->CreateNewGuid();
        SpecialNode->PostPlacedNewNode();
        SpecialNode->AllocateDefaultPins();
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), SpecialNode->NodeGuid.ToString());
        return ResultObj;
    }

    // Special case: Break Struct node
    if (FunctionName.Equals(TEXT("BreakStruct"), ESearchCase::IgnoreCase) && Params->HasField(TEXT("struct_type")))
    {
        FString StructTypeName = Params->GetStringField(TEXT("struct_type"));
        UScriptStruct* StructType = nullptr;
        // Try to find the struct by name (try F prefix and without)
        FString TryNames[] = { StructTypeName, FString::Printf(TEXT("F%s"), *StructTypeName) };
        for (const FString& Name : TryNames)
        {
            StructType = FindObject<UScriptStruct>(nullptr, *Name);
            if (StructType) break;
        }
        if (!StructType)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Struct type not found: %s"), *StructTypeName));
        }
        UK2Node_BreakStruct* BreakNode = NewObject<UK2Node_BreakStruct>(EventGraph);
        BreakNode->StructType = StructType;
        BreakNode->NodePosX = NodePosition.X;
        BreakNode->NodePosY = NodePosition.Y;
        EventGraph->AddNode(BreakNode, true);
        BreakNode->CreateNewGuid();
        BreakNode->PostPlacedNewNode();
        BreakNode->AllocateDefaultPins();
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("node_id"), BreakNode->NodeGuid.ToString());
        return ResultObj;
    }

    // Find the function
    UFunction* Function = nullptr;
    UK2Node_CallFunction* FunctionNode = nullptr;
    
    // Add extensive logging for debugging
    UE_LOG(LogTemp, Display, TEXT("Looking for function '%s' in target '%s'"), 
           *FunctionName, Target.IsEmpty() ? TEXT("Blueprint") : *Target);
    
    // Check if we have a target class specified
    if (!Target.IsEmpty())
    {
        // Try to find the target class using various paths
        TArray<FString> ClassPaths;
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildEnginePath(Target));
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildCorePath(Target));
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("Blueprints/%s.%s_C"), *Target, *Target)));
        ClassPaths.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("%s.%s_C"), *Target, *Target)));

        UClass* TargetClass = nullptr;
        for (const FString& ClassPath : ClassPaths)
        {
            TargetClass = LoadObject<UClass>(nullptr, *ClassPath);
            if (TargetClass)
            {
                break;
            }
        }

        // Try with U prefix if not found
        if (!TargetClass)
        {
            FString TargetWithPrefix = FString::Printf(TEXT("U%s"), *Target);
            TArray<FString> PrefixedPaths;
            PrefixedPaths.Add(FUnrealMCPCommonUtils::BuildEnginePath(TargetWithPrefix));
            PrefixedPaths.Add(FUnrealMCPCommonUtils::BuildCorePath(TargetWithPrefix));
            
            for (const FString& Path : PrefixedPaths)
            {
                TargetClass = LoadObject<UClass>(nullptr, *Path);
                if (TargetClass)
                {
                    break;
                }
            }
        }

        // For UGameplayStatics specific lookup
        if (!TargetClass && Target.Equals(TEXT("GameplayStatics")))
        {
            UE_LOG(LogTemp, Display, TEXT("Using special case handling for GameplayStatics::GetActorOfClass"));
            
            // Create the function node directly
            FunctionNode = NewObject<UK2Node_CallFunction>(EventGraph);
            if (FunctionNode)
            {
                // Direct setup for known function
                FunctionNode->FunctionReference.SetExternalMember(
                    FName(TEXT("GetActorOfClass")), 
                    TargetClass
                );
                
                FunctionNode->NodePosX = NodePosition.X;
                FunctionNode->NodePosY = NodePosition.Y;
                EventGraph->AddNode(FunctionNode);
                FunctionNode->CreateNewGuid();
                FunctionNode->PostPlacedNewNode();
                FunctionNode->AllocateDefaultPins();
                
                UE_LOG(LogTemp, Display, TEXT("Created GetActorOfClass node directly"));
                
                // List all pins
                for (UEdGraphPin* Pin : FunctionNode->Pins)
                {
                    UE_LOG(LogTemp, Display, TEXT("  - Pin: %s, Direction: %d, Category: %s"), 
                           *Pin->PinName.ToString(), (int32)Pin->Direction, *Pin->PinType.PinCategory.ToString());
                }
            }
        }
        
        // If we found a target class, look for the function there
        if (TargetClass)
        {
            UE_LOG(LogTemp, Display, TEXT("Looking for function '%s' in class '%s'"), 
                   *FunctionName, *TargetClass->GetName());
                   
            // First try exact name
            Function = TargetClass->FindFunctionByName(*FunctionName);
            
            // If not found, try class hierarchy
            UClass* CurrentClass = TargetClass;
            while (!Function && CurrentClass)
            {
                UE_LOG(LogTemp, Display, TEXT("Searching in class: %s"), *CurrentClass->GetName());
                
                // Try exact match
                Function = CurrentClass->FindFunctionByName(*FunctionName);
                
                // Try case-insensitive match
                if (!Function)
                {
                    for (TFieldIterator<UFunction> FuncIt(CurrentClass); FuncIt; ++FuncIt)
                    {
                        UFunction* AvailableFunc = *FuncIt;
                        UE_LOG(LogTemp, Display, TEXT("  - Available function: %s"), *AvailableFunc->GetName());
                        
                        if (AvailableFunc->GetName().Equals(FunctionName, ESearchCase::IgnoreCase))
                        {
                            UE_LOG(LogTemp, Display, TEXT("  - Found case-insensitive match: %s"), *AvailableFunc->GetName());
                            Function = AvailableFunc;
                            break;
                        }
                    }
                }
                
                // Move to parent class
                CurrentClass = CurrentClass->GetSuperClass();
            }
            
            // Special handling for known functions
            if (!Function)
            {
                if (TargetClass->GetName() == TEXT("GameplayStatics") && 
                    (FunctionName == TEXT("GetActorOfClass") || FunctionName.Equals(TEXT("GetActorOfClass"), ESearchCase::IgnoreCase)))
                {
                    UE_LOG(LogTemp, Display, TEXT("Using special case handling for GameplayStatics::GetActorOfClass"));
                    
                    // Create the function node directly
                    FunctionNode = NewObject<UK2Node_CallFunction>(EventGraph);
                    if (FunctionNode)
                    {
                        // Direct setup for known function
                        FunctionNode->FunctionReference.SetExternalMember(
                            FName(TEXT("GetActorOfClass")), 
                            TargetClass
                        );
                        
                        FunctionNode->NodePosX = NodePosition.X;
                        FunctionNode->NodePosY = NodePosition.Y;
                        EventGraph->AddNode(FunctionNode);
                        FunctionNode->CreateNewGuid();
                        FunctionNode->PostPlacedNewNode();
                        FunctionNode->AllocateDefaultPins();
                        
                        UE_LOG(LogTemp, Display, TEXT("Created GetActorOfClass node directly"));
                        
                        // List all pins
                        for (UEdGraphPin* Pin : FunctionNode->Pins)
                        {
                            UE_LOG(LogTemp, Display, TEXT("  - Pin: %s, Direction: %d, Category: %s"), 
                                   *Pin->PinName.ToString(), (int32)Pin->Direction, *Pin->PinType.PinCategory.ToString());
                        }
                    }
                }
            }
        }
    }
    
    // If we still haven't found the function, try in the blueprint's class
    if (!Function && !FunctionNode)
    {
        UE_LOG(LogTemp, Display, TEXT("Trying to find function in blueprint class"));
        Function = Blueprint->GeneratedClass->FindFunctionByName(*FunctionName);
    }
    
    // Create the function call node if we found the function
    if (Function && !FunctionNode)
    {
        FunctionNode = FUnrealMCPCommonUtils::CreateFunctionCallNode(EventGraph, Function, NodePosition);
    }
    
    if (!FunctionNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Function not found: %s in target %s"), *FunctionName, Target.IsEmpty() ? TEXT("Blueprint") : *Target));
    }

    // Set parameters if provided
    if (Params->HasField(TEXT("params")))
    {
        const TSharedPtr<FJsonObject>* ParamsObj;
        if (Params->TryGetObjectField(TEXT("params"), ParamsObj))
        {
            // Process parameters
            for (const TPair<FString, TSharedPtr<FJsonValue>>& Param : (*ParamsObj)->Values)
            {
                const FString& ParamName = Param.Key;
                const TSharedPtr<FJsonValue>& ParamValue = Param.Value;
                
                // Find the parameter pin
                UEdGraphPin* ParamPin = FUnrealMCPCommonUtils::FindPin(FunctionNode, ParamName, EGPD_Input);
                if (ParamPin)
                {
                    UE_LOG(LogTemp, Display, TEXT("Found parameter pin '%s' of category '%s'"), 
                           *ParamName, *ParamPin->PinType.PinCategory.ToString());
                    UE_LOG(LogTemp, Display, TEXT("  Current default value: '%s'"), *ParamPin->DefaultValue);
                    if (ParamPin->PinType.PinSubCategoryObject.IsValid())
                    {
                        UE_LOG(LogTemp, Display, TEXT("  Pin subcategory: '%s'"), 
                               *ParamPin->PinType.PinSubCategoryObject->GetName());
                    }
                    
                    // Set parameter based on type
                    if (ParamValue->Type == EJson::String)
                    {
                        FString StringVal = ParamValue->AsString();
                        UE_LOG(LogTemp, Display, TEXT("  Setting string parameter '%s' to: '%s'"), 
                               *ParamName, *StringVal);
                        
                        // Handle class reference parameters (e.g., ActorClass in GetActorOfClass)
                        if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Class)
                        {
                            // For class references, we require the exact class name with proper prefix
                            // - Actor classes must start with 'A' (e.g., ACameraActor)
                            // - Non-actor classes must start with 'U' (e.g., UObject)
                            const FString& ClassName = StringVal;
                            
                            // TODO: This likely won't work in UE5.5+, so don't rely on it.
                            UClass* Class = LoadObject<UClass>(nullptr, *ClassName);

                            if (!Class)
                            {
                                // Try with Engine module path
                                FString EngineClassName = FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
                                Class = LoadObject<UClass>(nullptr, *EngineClassName);
                                UE_LOG(LogTemp, Display, TEXT("Trying Engine module path: %s"), *EngineClassName);
                            }
                            
                            if (!Class)
                            {
                                UE_LOG(LogTemp, Error, TEXT("Failed to find class '%s'. Make sure to use the exact class name with proper prefix (A for actors, U for non-actors)"), *ClassName);
                                return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to find class '%s'"), *ClassName));
                            }

                            const UEdGraphSchema_K2* K2Schema = Cast<const UEdGraphSchema_K2>(EventGraph->GetSchema());
                            if (!K2Schema)
                            {
                                UE_LOG(LogTemp, Error, TEXT("Failed to get K2Schema"));
                                return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get K2Schema"));
                            }

                            K2Schema->TrySetDefaultObject(*ParamPin, Class);
                            if (ParamPin->DefaultObject != Class)
                            {
                                UE_LOG(LogTemp, Error, TEXT("Failed to set class reference for pin '%s' to '%s'"), *ParamPin->PinName.ToString(), *ClassName);
                                return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to set class reference for pin '%s'"), *ParamPin->PinName.ToString()));
                            }

                            UE_LOG(LogTemp, Log, TEXT("Successfully set class reference for pin '%s' to '%s'"), *ParamPin->PinName.ToString(), *ClassName);
                            continue;
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Int)
                        {
                            // Ensure we're using an integer value (no decimal)
                            int32 IntValue = FMath::RoundToInt(ParamValue->AsNumber());
                            ParamPin->DefaultValue = FString::FromInt(IntValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set integer parameter '%s' to: %d (string: '%s')"), 
                                   *ParamName, IntValue, *ParamPin->DefaultValue);
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Float)
                        {
                            // For other numeric types
                            float FloatValue = ParamValue->AsNumber();
                            ParamPin->DefaultValue = FString::SanitizeFloat(FloatValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set float parameter '%s' to: %f (string: '%s')"), 
                                   *ParamName, FloatValue, *ParamPin->DefaultValue);
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean)
                        {
                            bool BoolValue = ParamValue->AsBool();
                            ParamPin->DefaultValue = BoolValue ? TEXT("true") : TEXT("false");
                            UE_LOG(LogTemp, Display, TEXT("  Set boolean parameter '%s' to: %s"), 
                                   *ParamName, *ParamPin->DefaultValue);
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && ParamPin->PinType.PinSubCategoryObject == TBaseStructure<FVector>::Get())
                        {
                            // Handle array parameters - like Vector parameters
                            const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
                            if (ParamValue->TryGetArray(ArrayValue))
                            {
                                // Check if this could be a vector (array of 3 numbers)
                                if (ArrayValue->Num() == 3)
                                {
                                    // Create a proper vector string: (X=0.0,Y=0.0,Z=1000.0)
                                    float X = (*ArrayValue)[0]->AsNumber();
                                    float Y = (*ArrayValue)[1]->AsNumber();
                                    float Z = (*ArrayValue)[2]->AsNumber();
                                    
                                    FString VectorString = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), X, Y, Z);
                                    ParamPin->DefaultValue = VectorString;
                                    
                                    UE_LOG(LogTemp, Display, TEXT("  Set vector parameter '%s' to: %s"), 
                                           *ParamName, *VectorString);
                                    UE_LOG(LogTemp, Display, TEXT("  Final pin value: '%s'"), 
                                           *ParamPin->DefaultValue);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("Array parameter type not fully supported yet"));
                                }
                            }
                        }
                    }
                    else if (ParamValue->Type == EJson::Number)
                    {
                        // Handle integer vs float parameters correctly
                        if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Int)
                        {
                            // Ensure we're using an integer value (no decimal)
                            int32 IntValue = FMath::RoundToInt(ParamValue->AsNumber());
                            ParamPin->DefaultValue = FString::FromInt(IntValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set integer parameter '%s' to: %d (string: '%s')"), 
                                   *ParamName, IntValue, *ParamPin->DefaultValue);
                        }
                        else
                        {
                            // For other numeric types
                            float FloatValue = ParamValue->AsNumber();
                            ParamPin->DefaultValue = FString::SanitizeFloat(FloatValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set float parameter '%s' to: %f (string: '%s')"), 
                                   *ParamName, FloatValue, *ParamPin->DefaultValue);
                        }
                    }
                    else if (ParamValue->Type == EJson::Boolean)
                    {
                        bool BoolValue = ParamValue->AsBool();
                        ParamPin->DefaultValue = BoolValue ? TEXT("true") : TEXT("false");
                        UE_LOG(LogTemp, Display, TEXT("  Set boolean parameter '%s' to: %s"), 
                               *ParamName, *ParamPin->DefaultValue);
                    }
                    else if (ParamValue->Type == EJson::Array)
                    {
                        UE_LOG(LogTemp, Display, TEXT("  Processing array parameter '%s'"), *ParamName);
                        // Handle array parameters - like Vector parameters
                        const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
                        if (ParamValue->TryGetArray(ArrayValue))
                        {
                            // Check if this could be a vector (array of 3 numbers)
                            if (ArrayValue->Num() == 3 && 
                                (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct) &&
                                (ParamPin->PinType.PinSubCategoryObject == TBaseStructure<FVector>::Get()))
                            {
                                // Create a proper vector string: (X=0.0,Y=0.0,Z=1000.0)
                                float X = (*ArrayValue)[0]->AsNumber();
                                float Y = (*ArrayValue)[1]->AsNumber();
                                float Z = (*ArrayValue)[2]->AsNumber();
                                
                                FString VectorString = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), X, Y, Z);
                                ParamPin->DefaultValue = VectorString;
                                
                                UE_LOG(LogTemp, Display, TEXT("  Set vector parameter '%s' to: %s"), 
                                       *ParamName, *VectorString);
                                UE_LOG(LogTemp, Display, TEXT("  Final pin value: '%s'"), 
                                       *ParamPin->DefaultValue);
                            }
                            else
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Array parameter type not fully supported yet"));
                            }
                        }
                    }
                    // Add handling for other types as needed
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Parameter pin '%s' not found"), *ParamName);
                }
            }
        }
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), FunctionNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintVariable(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString VariableName;
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }

    FString VariableType;
    if (!Params->TryGetStringField(TEXT("variable_type"), VariableType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_type' parameter"));
    }

    // Get optional parameters
    bool IsExposed = false;
    if (Params->HasField(TEXT("is_exposed")))
    {
        IsExposed = Params->GetBoolField(TEXT("is_exposed"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Create variable based on type
    FEdGraphPinType PinType;
    bool bTypeResolved = false;

    auto SetPinTypeForCategory = [&](auto Category, UObject* SubCategoryObject = nullptr) {
        if constexpr (std::is_same_v<std::decay_t<decltype(Category)>, FName>) {
            PinType.PinCategory = Category;
        } else {
            PinType.PinCategory = FName(Category);
        }
        PinType.PinSubCategoryObject = SubCategoryObject;
        bTypeResolved = true;
    };

    FString TypeStr = VariableType;
    if (TypeStr.StartsWith(TEXT("/")))
    {
        TypeStr.RemoveFromStart(TEXT("/"));
    }
    TypeStr.TrimStartAndEndInline();

    // Handle array, set, map containers
    if (TypeStr.EndsWith(TEXT("[]"))) {
        // Array type
        FString InnerType = TypeStr.LeftChop(2);
        InnerType.TrimStartAndEndInline();
        // Recursively resolve inner type
        FEdGraphPinType InnerPinType;
        bool bInnerResolved = false;
        // Use a lambda for recursion
        auto ResolveType = [&](const FString& InType, FEdGraphPinType& OutPinType, bool& bResolved) {
            // Built-in types
            if (InType.Equals(TEXT("Float"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Float;
                bResolved = true;
            } else if (InType.Equals(TEXT("Boolean"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
                bResolved = true;
            } else if (InType.Equals(TEXT("Integer"), ESearchCase::IgnoreCase) || InType.Equals(TEXT("Int"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Int;
                bResolved = true;
            } else if (InType.Equals(TEXT("String"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_String;
                bResolved = true;
            } else if (InType.Equals(TEXT("Name"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Name;
                bResolved = true;
            } else if (InType.Equals(TEXT("Text"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Text;
                bResolved = true;
            } else if (InType.Equals(TEXT("Vector"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                OutPinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
                bResolved = true;
            } else if (InType.Equals(TEXT("Rotator"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                OutPinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
                bResolved = true;
            } else if (InType.Equals(TEXT("Transform"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                OutPinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
                bResolved = true;
            } else if (InType.Equals(TEXT("Color"), ESearchCase::IgnoreCase)) {
                OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                OutPinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
                bResolved = true;
            } else {
                // Try struct
                UScriptStruct* FoundStruct = nullptr;
                TArray<FString> StructNameVariations;
                StructNameVariations.Add(InType);
                StructNameVariations.Add(FString::Printf(TEXT("F%s"), *InType));
                StructNameVariations.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("Blueprints/%s.%s"), *InType, *InType)));
                StructNameVariations.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("DataStructures/%s.%s"), *InType, *InType)));
                StructNameVariations.Add(FUnrealMCPCommonUtils::BuildEnginePath(InType));
                for (const FString& StructVariation : StructNameVariations) {
                    FoundStruct = LoadObject<UScriptStruct>(nullptr, *StructVariation);
                    if (FoundStruct) break;
                }
                if (!FoundStruct) {
                    for (const FString& StructVariation : StructNameVariations) {
                        FoundStruct = LoadObject<UScriptStruct>(nullptr, *StructVariation);
                        if (FoundStruct) break;
                    }
                }
                if (FoundStruct) {
                    OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    OutPinType.PinSubCategoryObject = FoundStruct;
                    bResolved = true;
                } else {
                    // Try enum
                    UEnum* FoundEnum = LoadObject<UEnum>(nullptr, *InType);
                    if (!FoundEnum) {
                        FoundEnum = LoadObject<UEnum>(nullptr, *InType);
                    }
                    if (FoundEnum) {
                        OutPinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
                        OutPinType.PinSubCategoryObject = FoundEnum;
                        bResolved = true;
                    } else {
                        // Try object/class
                        UClass* FoundClass = nullptr;
                        
                        // Clean up the path by removing any redundant slashes
                        FString CleanPath = InType;
                        if (CleanPath.StartsWith(TEXT("/")))
                        {
                            CleanPath.RemoveFromStart(TEXT("/"));
                        }
                        CleanPath.TrimStartAndEndInline();
                        
                        // Handle Game prefix variations
                        if (!CleanPath.StartsWith(TEXT("/")))
                        {
                            if (CleanPath.StartsWith(TEXT("Game/")))
                            {
                                CleanPath = FString::Printf(TEXT("/%s"), *CleanPath);
                            }
                            else
                            {
                                CleanPath = FString::Printf(TEXT("/Game/%s"), *CleanPath);
                            }
                        }
                        
                        // Try direct path first
                        FString DirectPath = FString::Printf(TEXT("%s.%s_C"), *CleanPath, *FPaths::GetBaseFilename(CleanPath));
                        UE_LOG(LogTemp, Display, TEXT("Trying direct path: %s"), *DirectPath);
                        FoundClass = LoadClass<UObject>(nullptr, *DirectPath);
                        
                        // If not found, try standard class loading
                        if (!FoundClass)
                        {
                            FoundClass = LoadObject<UClass>(nullptr, *CleanPath);
                        }
                        
                        // Try engine path if still not found
                        if (!FoundClass)
                        {
                            FString EngineClassName = FUnrealMCPCommonUtils::BuildEnginePath(CleanPath);
                            FoundClass = LoadObject<UClass>(nullptr, *EngineClassName);
                        }
                        
                        if (FoundClass)
                        {
                            OutPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
                            OutPinType.PinSubCategoryObject = FoundClass;
                            bResolved = true;
                        }
                    }
                }
            }
            
            // Always return success, the bResolved flag will indicate if the type was actually resolved
            return TSharedPtr<FJsonObject>();
        };

        ResolveType(InnerType, InnerPinType, bInnerResolved);
        if (bInnerResolved) {
            PinType = InnerPinType;
            PinType.ContainerType = EPinContainerType::Array;
            bTypeResolved = true;
        }
    } else {
        // Built-in types
        if (TypeStr.Equals(TEXT("Float"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Float);
        } else if (TypeStr.Equals(TEXT("Boolean"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Boolean);
        } else if (TypeStr.Equals(TEXT("Integer"), ESearchCase::IgnoreCase) || TypeStr.Equals(TEXT("Int"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Int);
        } else if (TypeStr.Equals(TEXT("String"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_String);
        } else if (TypeStr.Equals(TEXT("Name"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Name);
        } else if (TypeStr.Equals(TEXT("Text"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Text);
        } else if (TypeStr.Equals(TEXT("Vector"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get());
        } else if (TypeStr.Equals(TEXT("Rotator"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Struct, TBaseStructure<FRotator>::Get());
        } else if (TypeStr.Equals(TEXT("Transform"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Struct, TBaseStructure<FTransform>::Get());
        } else if (TypeStr.Equals(TEXT("Color"), ESearchCase::IgnoreCase)) {
            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Struct, TBaseStructure<FLinearColor>::Get());
        } else {
            // Try struct
            UScriptStruct* FoundStruct = nullptr;
            TArray<FString> StructNameVariations;
            StructNameVariations.Add(TypeStr);
            StructNameVariations.Add(FString::Printf(TEXT("F%s"), *TypeStr));
            StructNameVariations.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("Blueprints/%s.%s"), *TypeStr, *TypeStr)));
            StructNameVariations.Add(FUnrealMCPCommonUtils::BuildGamePath(FString::Printf(TEXT("DataStructures/%s.%s"), *TypeStr, *TypeStr)));
            StructNameVariations.Add(FUnrealMCPCommonUtils::BuildEnginePath(TypeStr));
            for (const FString& StructVariation : StructNameVariations) {
                FoundStruct = LoadObject<UScriptStruct>(nullptr, *StructVariation);
                if (FoundStruct) break;
            }
            if (!FoundStruct) {
                for (const FString& StructVariation : StructNameVariations) {
                    FoundStruct = LoadObject<UScriptStruct>(nullptr, *StructVariation);
                    if (FoundStruct) break;
                }
            }
            if (FoundStruct) {
                SetPinTypeForCategory(UEdGraphSchema_K2::PC_Struct, FoundStruct);
            } else {
                // Try object/class
                UClass* FoundClass = nullptr;
                
                // Clean up the path by removing any redundant slashes
                FString CleanPath = TypeStr;
                if (CleanPath.StartsWith(TEXT("/")))
                {
                    CleanPath.RemoveFromStart(TEXT("/"));
                }
                CleanPath.TrimStartAndEndInline();
                
                // Handle Game prefix variations
                if (!CleanPath.StartsWith(TEXT("/")))
                {
                    if (CleanPath.StartsWith(TEXT("Game/")))
                    {
                        CleanPath = FString::Printf(TEXT("/%s"), *CleanPath);
                    }
                    else
                    {
                        CleanPath = FString::Printf(TEXT("/Game/%s"), *CleanPath);
                    }
                }
                
                // Try direct path first
                FString DirectPath = FString::Printf(TEXT("%s.%s_C"), *CleanPath, *FPaths::GetBaseFilename(CleanPath));
                UE_LOG(LogTemp, Display, TEXT("Trying direct path: %s"), *DirectPath);
                FoundClass = LoadClass<UObject>(nullptr, *DirectPath);
                
                // If not found, try standard class loading
                if (!FoundClass)
                {
                    FoundClass = LoadObject<UClass>(nullptr, *CleanPath);
                }
                
                // Try engine path if still not found
                if (!FoundClass)
                {
                    FString EngineClassName = FUnrealMCPCommonUtils::BuildEnginePath(CleanPath);
                    FoundClass = LoadObject<UClass>(nullptr, *EngineClassName);
                }
                
                if (FoundClass)
                {
                    SetPinTypeForCategory(UEdGraphSchema_K2::PC_Object, FoundClass);
                }
                else
                {
                    // Special handling for DataTable references
                    if (TypeStr.Contains(TEXT("Game/")))
                    {
                        // Ensure path starts with a forward slash
                        FString DataTablePath = TypeStr;
                        if (!DataTablePath.StartsWith(TEXT("/")))
                        {
                            DataTablePath = TEXT("/") + DataTablePath;
                        }
                        
                        // Try to load the DataTable asset
                        UDataTable* DataTableAsset = LoadObject<UDataTable>(nullptr, *DataTablePath);
                        if (DataTableAsset)
                        {
                            // For DataTables, we use Object reference with DataTable class
                            UClass* DataTableClass = UDataTable::StaticClass();
                            SetPinTypeForCategory(UEdGraphSchema_K2::PC_Object, DataTableClass);
                        }
                        else
                        {
                            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Could not find DataTable asset: %s"), *DataTablePath));
                        }
                    }
                    else
                    {
                        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Could not find class for type: %s"), *TypeStr));
                    }
                }
            }
        }
    }
    if (!bTypeResolved) {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Could not resolve variable type: %s"), *VariableType));
    }

    // Create the variable
    FBlueprintEditorUtils::AddMemberVariable(Blueprint, FName(*VariableName), PinType);

    // Set variable properties
    FBPVariableDescription* NewVar = nullptr;
    for (FBPVariableDescription& Variable : Blueprint->NewVariables)
    {
        if (Variable.VarName == FName(*VariableName))
        {
            NewVar = &Variable;
            break;
        }
    }

    if (NewVar)
    {
        // Set exposure in editor
        if (IsExposed)
        {
            NewVar->PropertyFlags |= CPF_Edit;
        }
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("variable_name"), VariableName);
    ResultObj->SetStringField(TEXT("variable_type"), VariableType);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintInputActionNode(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActionName;
    if (!Params->TryGetStringField(TEXT("action_name"), ActionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'action_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the input action node
    UK2Node_InputAction* InputActionNode = FUnrealMCPCommonUtils::CreateInputActionNode(EventGraph, ActionName, NodePosition);
    if (!InputActionNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create input action node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), InputActionNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintSelfReference(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the self node
    UK2Node_Self* SelfNode = FUnrealMCPCommonUtils::CreateSelfReferenceNode(EventGraph, NodePosition);
    if (!SelfNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create self node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), SelfNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleFindBlueprintNodes(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    // Get optional node_type parameter
    FString NodeType;
    if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
    {
        NodeType = TEXT("All"); // Default to finding all nodes
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create a JSON array for the node GUIDs
    TArray<TSharedPtr<FJsonValue>> NodeGuidArray;
    
    UE_LOG(LogTemp, Display, TEXT("Searching for nodes of type '%s' in blueprint '%s'"), *NodeType, *BlueprintName);
    UE_LOG(LogTemp, Display, TEXT("Total nodes in graph: %d"), EventGraph->Nodes.Num());
    
    // Filter nodes by the requested type
    if (NodeType.Equals(TEXT("Event"), ESearchCase::IgnoreCase))
    {
        // Get optional event_name parameter for specific event filtering
        FString EventName;
        bool bHasSpecificEvent = Params->TryGetStringField(TEXT("event_name"), EventName);
        
        UE_LOG(LogTemp, Display, TEXT("Looking for Event nodes%s"), 
               bHasSpecificEvent ? *FString::Printf(TEXT(" with name '%s'"), *EventName) : TEXT(" (all events)"));
        
        // Look for event nodes
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node);
            if (EventNode)
            {
                FString EventNodeName = EventNode->EventReference.GetMemberName().ToString();
                UE_LOG(LogTemp, Display, TEXT("Found event node: %s"), *EventNodeName);
                
                // If specific event name is provided, filter by it
                if (bHasSpecificEvent)
                {
                    if (EventNodeName.Equals(EventName, ESearchCase::IgnoreCase))
                    {
                        UE_LOG(LogTemp, Display, TEXT("Event node matches filter: %s"), *EventNode->NodeGuid.ToString());
                        NodeGuidArray.Add(MakeShared<FJsonValueString>(EventNode->NodeGuid.ToString()));
                    }
                }
                else
                {
                    // Add all event nodes
                    UE_LOG(LogTemp, Display, TEXT("Adding event node: %s"), *EventNode->NodeGuid.ToString());
                    NodeGuidArray.Add(MakeShared<FJsonValueString>(EventNode->NodeGuid.ToString()));
                }
            }
        }
    }
    else if (NodeType.Equals(TEXT("Function"), ESearchCase::IgnoreCase))
    {
        // Look for function call nodes
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            UK2Node_CallFunction* FunctionNode = Cast<UK2Node_CallFunction>(Node);
            if (FunctionNode)
            {
                UE_LOG(LogTemp, Display, TEXT("Found function node: %s"), *FunctionNode->NodeGuid.ToString());
                NodeGuidArray.Add(MakeShared<FJsonValueString>(FunctionNode->NodeGuid.ToString()));
            }
        }
    }
    else if (NodeType.Equals(TEXT("Variable"), ESearchCase::IgnoreCase))
    {
        // Look for variable nodes (get/set)
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            UK2Node_VariableGet* VarGetNode = Cast<UK2Node_VariableGet>(Node);
            if (VarGetNode)
            {
                UE_LOG(LogTemp, Display, TEXT("Found variable get node: %s"), *VarGetNode->NodeGuid.ToString());
                NodeGuidArray.Add(MakeShared<FJsonValueString>(VarGetNode->NodeGuid.ToString()));
                continue;
            }
            
            UK2Node_VariableSet* VarSetNode = Cast<UK2Node_VariableSet>(Node);
            if (VarSetNode)
            {
                UE_LOG(LogTemp, Display, TEXT("Found variable set node: %s"), *VarSetNode->NodeGuid.ToString());
                NodeGuidArray.Add(MakeShared<FJsonValueString>(VarSetNode->NodeGuid.ToString()));
            }
        }
    }
    else if (NodeType.Equals(TEXT("All"), ESearchCase::IgnoreCase))
    {
        // Return all nodes
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            if (Node)
            {
                UE_LOG(LogTemp, Display, TEXT("Found node: %s (Type: %s)"), 
                       *Node->NodeGuid.ToString(), 
                       *Node->GetClass()->GetName());
                NodeGuidArray.Add(MakeShared<FJsonValueString>(Node->NodeGuid.ToString()));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unsupported node type: %s"), *NodeType);
        // For unsupported types, we'll still search by class name
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            if (Node && Node->GetClass()->GetName().Contains(NodeType))
            {
                UE_LOG(LogTemp, Display, TEXT("Found node matching type '%s': %s"), *NodeType, *Node->NodeGuid.ToString());
                NodeGuidArray.Add(MakeShared<FJsonValueString>(Node->NodeGuid.ToString()));
            }
        }
    }
    
    UE_LOG(LogTemp, Display, TEXT("Found %d matching nodes"), NodeGuidArray.Num());
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("node_guids"), NodeGuidArray);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintCustomEventNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));

    FString EventName;
    if (!Params->TryGetStringField(TEXT("event_name"), EventName))
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'event_name' parameter"));

    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));

    // Check for existing custom event node
    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        UK2Node_CustomEvent* CustomEventNode = Cast<UK2Node_CustomEvent>(Node);
        if (CustomEventNode && CustomEventNode->CustomFunctionName == FName(*EventName))
        {
            TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
            Obj->SetStringField(TEXT("node_id"), CustomEventNode->NodeGuid.ToString());
            Obj->SetStringField(TEXT("event_name"), EventName);
            return Obj;
        }
    }

    // Create new custom event node
    UK2Node_CustomEvent* NewEventNode = NewObject<UK2Node_CustomEvent>(EventGraph);
    NewEventNode->CustomFunctionName = FName(*EventName);
    NewEventNode->NodePosX = NodePosition.X;
    NewEventNode->NodePosY = NodePosition.Y;
    EventGraph->AddNode(NewEventNode, true);
    NewEventNode->CreateNewGuid();
    NewEventNode->PostPlacedNewNode();
    NewEventNode->AllocateDefaultPins();

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), NewEventNode->NodeGuid.ToString());
    ResultObj->SetStringField(TEXT("event_name"), EventName);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleGetVariableInfo(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }
    FString VariableName;
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }
    FString TypeString = TEXT("Unknown");
    for (const FBPVariableDescription& Var : Blueprint->NewVariables)
    {
        if (Var.VarName == FName(*VariableName))
        {
            if (Var.VarType.PinCategory == UEdGraphSchema_K2::PC_Struct && Var.VarType.PinSubCategoryObject.IsValid())
            {
                // User-defined struct: return full asset path
                UScriptStruct* StructObj = Cast<UScriptStruct>(Var.VarType.PinSubCategoryObject.Get());
                if (StructObj)
                {
                    FString StructPath = StructObj->GetPathName();
                    // Remove dot notation if present (e.g., /Game/Blueprints/DialogueData.DialogueData -> /Game/Blueprints/DialogueData)
                    int32 DotIndex;
                    if (StructPath.FindChar('.', DotIndex))
                    {
                        StructPath = StructPath.Left(DotIndex);
                    }
                    TypeString = StructPath;
                }
            }
            else if (!Var.VarType.PinCategory.IsNone())
            {
                TypeString = Var.VarType.PinCategory.ToString();
            }
            else
            {
                TypeString = Var.FriendlyName;
            }
            break;
        }
    }
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("variable_type"), TypeString);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddEnhancedInputActionNode(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActionPath;
    if (!Params->TryGetStringField(TEXT("action_path"), ActionPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'action_path' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Load the Input Action asset
    UInputAction* InputAction = Cast<UInputAction>(UEditorAssetLibrary::LoadAsset(ActionPath));
    if (!InputAction)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load Input Action: %s"), *ActionPath));
    }

    // Check if we already have an Enhanced Input Action node for this action
    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        // In UE5, Enhanced Input uses specific node types
        // We need to check for the appropriate Enhanced Input event node type
        if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
        {
            // Check if this is an Enhanced Input event for our action
            if (EventNode->EventReference.GetMemberName().ToString().Contains(InputAction->GetName()))
            {
                TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
                ResultObj->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
                ResultObj->SetStringField(TEXT("action_path"), ActionPath);
                ResultObj->SetBoolField(TEXT("already_exists"), true);
                return ResultObj;
            }
        }
    }

    // For Enhanced Input in UE5, we need to create an Input Action event node
    // This is typically done differently than legacy input actions
    UK2Node_Event* NewEventNode = NewObject<UK2Node_Event>(EventGraph);
    if (!NewEventNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Enhanced Input Action event node"));
    }

    // Set up the event reference for Enhanced Input
    // The exact implementation depends on how UE5's Enhanced Input system works
    FName EventName = FName(*FString::Printf(TEXT("InputAction_%s"), *InputAction->GetName()));
    NewEventNode->EventReference.SetFromField<UFunction>(
        FindUField<UFunction>(AActor::StaticClass(), EventName), false);

    NewEventNode->NodePosX = NodePosition.X;
    NewEventNode->NodePosY = NodePosition.Y;
    EventGraph->AddNode(NewEventNode, true);
    NewEventNode->CreateNewGuid();
    NewEventNode->PostPlacedNewNode();
    NewEventNode->AllocateDefaultPins();

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), NewEventNode->NodeGuid.ToString());
    ResultObj->SetStringField(TEXT("action_path"), ActionPath);
    ResultObj->SetBoolField(TEXT("already_exists"), false);
    return ResultObj;
} 