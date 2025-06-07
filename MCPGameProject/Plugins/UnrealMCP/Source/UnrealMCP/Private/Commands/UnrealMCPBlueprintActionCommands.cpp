#include "Commands/UnrealMCPBlueprintActionCommands.h"
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

FString UUnrealMCPBlueprintActionCommands::GetActionsForPin(const FString& PinType, const FString& PinSubCategory)
{
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> ActionsArray;
    
    // For now, provide some basic example actions based on pin type
    TArray<FString> ExampleActions;
    
    if (PinType.Equals(TEXT("object"), ESearchCase::IgnoreCase))
    {
        ExampleActions.Add(TEXT("Cast to"));
        ExampleActions.Add(TEXT("Is Valid"));
        ExampleActions.Add(TEXT("Get Class"));
        ExampleActions.Add(TEXT("Get Name"));
        if (PinSubCategory.Equals(TEXT("Actor"), ESearchCase::IgnoreCase))
        {
            ExampleActions.Add(TEXT("Get Actor Location"));
            ExampleActions.Add(TEXT("Set Actor Location"));
            ExampleActions.Add(TEXT("Destroy Actor"));
        }
        else if (PinSubCategory.Equals(TEXT("Character"), ESearchCase::IgnoreCase))
        {
            ExampleActions.Add(TEXT("Get Velocity"));
            ExampleActions.Add(TEXT("Jump"));
            ExampleActions.Add(TEXT("Get Movement Component"));
        }
        else if (PinSubCategory.Equals(TEXT("PlayerController"), ESearchCase::IgnoreCase))
        {
            ExampleActions.Add(TEXT("Get Pawn"));
            ExampleActions.Add(TEXT("Possess"));
            ExampleActions.Add(TEXT("UnPossess"));
        }
    }
    else if (PinType.Equals(TEXT("float"), ESearchCase::IgnoreCase))
    {
        ExampleActions.Add(TEXT("Add"));
        ExampleActions.Add(TEXT("Subtract"));
        ExampleActions.Add(TEXT("Multiply"));
        ExampleActions.Add(TEXT("Divide"));
        ExampleActions.Add(TEXT("Clamp"));
        ExampleActions.Add(TEXT("Lerp"));
    }
    else if (PinType.Equals(TEXT("int"), ESearchCase::IgnoreCase) || PinType.Equals(TEXT("integer"), ESearchCase::IgnoreCase))
    {
        ExampleActions.Add(TEXT("Add"));
        ExampleActions.Add(TEXT("Subtract"));
        ExampleActions.Add(TEXT("Multiply"));
        ExampleActions.Add(TEXT("Divide"));
        ExampleActions.Add(TEXT("Modulo"));
        ExampleActions.Add(TEXT("Random Integer"));
    }
    else if (PinType.Equals(TEXT("bool"), ESearchCase::IgnoreCase) || PinType.Equals(TEXT("boolean"), ESearchCase::IgnoreCase))
    {
        ExampleActions.Add(TEXT("AND"));
        ExampleActions.Add(TEXT("OR"));
        ExampleActions.Add(TEXT("NOT"));
        ExampleActions.Add(TEXT("Branch"));
    }
    else if (PinType.Equals(TEXT("string"), ESearchCase::IgnoreCase))
    {
        ExampleActions.Add(TEXT("Append"));
        ExampleActions.Add(TEXT("Contains"));
        ExampleActions.Add(TEXT("Get Length"));
        ExampleActions.Add(TEXT("To Upper"));
        ExampleActions.Add(TEXT("To Lower"));
    }
    else if (PinType.Equals(TEXT("vector"), ESearchCase::IgnoreCase))
    {
        ExampleActions.Add(TEXT("Add"));
        ExampleActions.Add(TEXT("Subtract"));
        ExampleActions.Add(TEXT("Multiply"));
        ExampleActions.Add(TEXT("Normalize"));
        ExampleActions.Add(TEXT("Length"));
        ExampleActions.Add(TEXT("Cross Product"));
        ExampleActions.Add(TEXT("Dot Product"));
    }
    else
    {
        // Generic actions for unknown pin types
        ExampleActions.Add(TEXT("Set"));
        ExampleActions.Add(TEXT("Get"));
        ExampleActions.Add(TEXT("Convert"));
    }
    
    // Create action objects
    for (const FString& ActionName : ExampleActions)
    {
        TSharedPtr<FJsonObject> ActionObj = MakeShared<FJsonObject>();
        ActionObj->SetStringField(TEXT("title"), ActionName);
        ActionObj->SetStringField(TEXT("tooltip"), FString::Printf(TEXT("%s for %s pins"), *ActionName, *PinType));
        ActionObj->SetStringField(TEXT("category"), TEXT("Math"));
        ActionObj->SetStringField(TEXT("keywords"), FString::Printf(TEXT("%s %s"), *PinType, *ActionName));
        
        ActionsArray.Add(MakeShared<FJsonValueObject>(ActionObj));
    }
    
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("pin_type"), PinType);
    ResultObj->SetStringField(TEXT("pin_subcategory"), PinSubCategory);
    ResultObj->SetArrayField(TEXT("actions"), ActionsArray);
    ResultObj->SetNumberField(TEXT("action_count"), ActionsArray.Num());
    ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Found %d example actions for pin type '%s'"), ActionsArray.Num(), *PinType));
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
    
    return OutputString;
}

FString UUnrealMCPBlueprintActionCommands::GetActionsForClass(const FString& ClassName)
{
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> ActionsArray;
    
    // Find the class by name
    UClass* TargetClass = UClass::TryFindTypeSlow<UClass>(ClassName);
    if (!TargetClass)
    {
        // Try with different common class names
        FString TestClassName = ClassName;
        if (!TestClassName.StartsWith(TEXT("U")) && !TestClassName.StartsWith(TEXT("A")) && !TestClassName.StartsWith(TEXT("F")))
        {
            // Try with A prefix for Actor classes
            TestClassName = TEXT("A") + ClassName;
            TargetClass = UClass::TryFindTypeSlow<UClass>(TestClassName);
            
            if (!TargetClass)
            {
                // Try with U prefix for UObject classes
                TestClassName = TEXT("U") + ClassName;
                TargetClass = UClass::TryFindTypeSlow<UClass>(TestClassName);
            }
        }
    }
    
    if (TargetClass)
    {
        // Generate example actions based on class type
        TArray<FString> ClassActions;
        
        if (TargetClass->IsChildOf<AActor>())
        {
            ClassActions.Add(TEXT("Get Actor Location"));
            ClassActions.Add(TEXT("Set Actor Location"));
            ClassActions.Add(TEXT("Get Actor Rotation"));
            ClassActions.Add(TEXT("Set Actor Rotation"));
            ClassActions.Add(TEXT("Destroy Actor"));
            ClassActions.Add(TEXT("Get Owner"));
            ClassActions.Add(TEXT("Get World"));
        }
        
        if (TargetClass->IsChildOf<APawn>())
        {
            ClassActions.Add(TEXT("Get Controller"));
            ClassActions.Add(TEXT("Get Movement Component"));
            ClassActions.Add(TEXT("Add Movement Input"));
        }
        
        if (TargetClass->IsChildOf<ACharacter>())
        {
            ClassActions.Add(TEXT("Jump"));
            ClassActions.Add(TEXT("Stop Jumping"));
            ClassActions.Add(TEXT("Get Velocity"));
            ClassActions.Add(TEXT("Get Character Movement"));
        }
        
        if (TargetClass->IsChildOf<APlayerController>())
        {
            ClassActions.Add(TEXT("Get Pawn"));
            ClassActions.Add(TEXT("Possess"));
            ClassActions.Add(TEXT("UnPossess"));
            ClassActions.Add(TEXT("Get Player State"));
        }
        
        if (TargetClass->IsChildOf<UActorComponent>())
        {
            ClassActions.Add(TEXT("Get Owner"));
            ClassActions.Add(TEXT("Get World"));
            ClassActions.Add(TEXT("Set Active"));
        }
        
        // Add some generic UObject actions
        ClassActions.Add(TEXT("Get Class"));
        ClassActions.Add(TEXT("Get Name"));
        ClassActions.Add(TEXT("Is Valid"));
        
        // Create action objects
        for (const FString& ActionName : ClassActions)
        {
            TSharedPtr<FJsonObject> ActionObj = MakeShared<FJsonObject>();
            ActionObj->SetStringField(TEXT("title"), ActionName);
            ActionObj->SetStringField(TEXT("tooltip"), FString::Printf(TEXT("%s function for %s"), *ActionName, *ClassName));
            ActionObj->SetStringField(TEXT("category"), TEXT("Call Function"));
            ActionObj->SetStringField(TEXT("keywords"), FString::Printf(TEXT("%s %s"), *ClassName, *ActionName));
            
            ActionsArray.Add(MakeShared<FJsonValueObject>(ActionObj));
        }
        
        ResultObj->SetBoolField(TEXT("success"), true);
        ResultObj->SetStringField(TEXT("class_name"), ClassName);
        ResultObj->SetArrayField(TEXT("actions"), ActionsArray);
        ResultObj->SetNumberField(TEXT("action_count"), ActionsArray.Num());
        ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Found %d example actions for class '%s'"), ActionsArray.Num(), *ClassName));
    }
    else
    {
        // Class not found
        ResultObj->SetBoolField(TEXT("success"), false);
        ResultObj->SetStringField(TEXT("class_name"), ClassName);
        ResultObj->SetArrayField(TEXT("actions"), ActionsArray);
        ResultObj->SetNumberField(TEXT("action_count"), 0);
        ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Class '%s' not found"), *ClassName));
    }
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
    
    return OutputString;
}

FString UUnrealMCPBlueprintActionCommands::GetActionsForClassHierarchy(const FString& ClassName)
{
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> ActionsArray;
    TArray<TSharedPtr<FJsonValue>> HierarchyArray;
    TMap<FString, int32> CategoryCounts;
    
    // Find the class by name
    UClass* TargetClass = UClass::TryFindTypeSlow<UClass>(ClassName);
    if (!TargetClass)
    {
        // Try with different common class names
        FString TestClassName = ClassName;
        if (!TestClassName.StartsWith(TEXT("U")) && !TestClassName.StartsWith(TEXT("A")) && !TestClassName.StartsWith(TEXT("F")))
        {
            // Try with A prefix for Actor classes
            TestClassName = TEXT("A") + ClassName;
            TargetClass = UClass::TryFindTypeSlow<UClass>(TestClassName);
            
            if (!TargetClass)
            {
                // Try with U prefix for UObject classes
                TestClassName = TEXT("U") + ClassName;
                TargetClass = UClass::TryFindTypeSlow<UClass>(TestClassName);
            }
        }
    }
    
    if (TargetClass)
    {
        // Build class hierarchy
        TArray<UClass*> ClassHierarchy;
        UClass* CurrentClass = TargetClass;
        while (CurrentClass)
        {
            ClassHierarchy.Add(CurrentClass);
            HierarchyArray.Add(MakeShared<FJsonValueString>(CurrentClass->GetName()));
            CurrentClass = CurrentClass->GetSuperClass();
        }
        
        // Generate comprehensive actions based on entire hierarchy
        TArray<FString> HierarchyActions;
        
        // Add actions for each class in hierarchy
        for (UClass* HierarchyClass : ClassHierarchy)
        {
            if (HierarchyClass->IsChildOf<AActor>())
            {
                HierarchyActions.Add(TEXT("Get Actor Location"));
                HierarchyActions.Add(TEXT("Set Actor Location"));
                HierarchyActions.Add(TEXT("Get Actor Rotation"));
                HierarchyActions.Add(TEXT("Set Actor Rotation"));
                HierarchyActions.Add(TEXT("Destroy Actor"));
                HierarchyActions.Add(TEXT("Get Owner"));
                HierarchyActions.Add(TEXT("Get World"));
                HierarchyActions.Add(TEXT("Set Actor Hidden In Game"));
                HierarchyActions.Add(TEXT("Set Actor Enable Collision"));
            }
            
            if (HierarchyClass->IsChildOf<APawn>())
            {
                HierarchyActions.Add(TEXT("Get Controller"));
                HierarchyActions.Add(TEXT("Get Movement Component"));
                HierarchyActions.Add(TEXT("Add Movement Input"));
                HierarchyActions.Add(TEXT("Add Controller Pitch Input"));
                HierarchyActions.Add(TEXT("Add Controller Yaw Input"));
            }
            
            if (HierarchyClass->IsChildOf<ACharacter>())
            {
                HierarchyActions.Add(TEXT("Jump"));
                HierarchyActions.Add(TEXT("Stop Jumping"));
                HierarchyActions.Add(TEXT("Get Velocity"));
                HierarchyActions.Add(TEXT("Get Character Movement"));
                HierarchyActions.Add(TEXT("Launch Character"));
            }
            
            if (HierarchyClass->IsChildOf<APlayerController>())
            {
                HierarchyActions.Add(TEXT("Get Pawn"));
                HierarchyActions.Add(TEXT("Possess"));
                HierarchyActions.Add(TEXT("UnPossess"));
                HierarchyActions.Add(TEXT("Get Player State"));
                HierarchyActions.Add(TEXT("Client Travel"));
            }
        }
        
        // Add generic UObject actions
        HierarchyActions.Add(TEXT("Get Class"));
        HierarchyActions.Add(TEXT("Get Name"));
        HierarchyActions.Add(TEXT("Is Valid"));
        HierarchyActions.Add(TEXT("Get Outer"));
        
        // Remove duplicates
        TSet<FString> UniqueActions(HierarchyActions);
        HierarchyActions = UniqueActions.Array();
        
        // Create action objects and count categories
        for (const FString& ActionName : HierarchyActions)
        {
            FString CategoryName;
            if (ActionName.Contains(TEXT("Get")))
            {
                CategoryName = TEXT("Getter");
            }
            else if (ActionName.Contains(TEXT("Set")))
            {
                CategoryName = TEXT("Setter");
            }
            else if (ActionName.Contains(TEXT("Add")))
            {
                CategoryName = TEXT("Input");
            }
            else
            {
                CategoryName = TEXT("Function");
            }
            
            CategoryCounts.FindOrAdd(CategoryName)++;
            
            TSharedPtr<FJsonObject> ActionObj = MakeShared<FJsonObject>();
            ActionObj->SetStringField(TEXT("title"), ActionName);
            ActionObj->SetStringField(TEXT("tooltip"), FString::Printf(TEXT("%s function for %s hierarchy"), *ActionName, *ClassName));
            ActionObj->SetStringField(TEXT("category"), CategoryName);
            ActionObj->SetStringField(TEXT("keywords"), FString::Printf(TEXT("%s %s"), *ClassName, *ActionName));
            
            ActionsArray.Add(MakeShared<FJsonValueObject>(ActionObj));
        }
        
        // Build category counts object
        TSharedPtr<FJsonObject> CategoryCountsObj = MakeShared<FJsonObject>();
        for (const auto& CountPair : CategoryCounts)
        {
            CategoryCountsObj->SetNumberField(CountPair.Key, CountPair.Value);
        }
        
        ResultObj->SetBoolField(TEXT("success"), true);
        ResultObj->SetStringField(TEXT("class_name"), ClassName);
        ResultObj->SetArrayField(TEXT("actions"), ActionsArray);
        ResultObj->SetArrayField(TEXT("class_hierarchy"), HierarchyArray);
        ResultObj->SetObjectField(TEXT("category_counts"), CategoryCountsObj);
        ResultObj->SetNumberField(TEXT("action_count"), ActionsArray.Num());
        ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Found %d actions for class hierarchy of '%s'"), ActionsArray.Num(), *ClassName));
    }
    else
    {
        // Class not found
        ResultObj->SetBoolField(TEXT("success"), false);
        ResultObj->SetStringField(TEXT("class_name"), ClassName);
        ResultObj->SetArrayField(TEXT("actions"), ActionsArray);
        ResultObj->SetArrayField(TEXT("class_hierarchy"), HierarchyArray);
        ResultObj->SetObjectField(TEXT("category_counts"), MakeShared<FJsonObject>());
        ResultObj->SetNumberField(TEXT("action_count"), 0);
        ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Class '%s' not found"), *ClassName));
    }
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
    
    return OutputString;
}

FString UUnrealMCPBlueprintActionCommands::GetNodePinInfo(const FString& NodeName, const FString& PinName)
{
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    
    // Define known node pin information
    TMap<FString, TMap<FString, TSharedPtr<FJsonObject>>> NodePinDatabase;
    
    // Create Widget node
    TMap<FString, TSharedPtr<FJsonObject>> CreateWidgetPins;
    
    auto ClassPin = MakeShared<FJsonObject>();
    ClassPin->SetStringField(TEXT("pin_type"), TEXT("class"));
    ClassPin->SetStringField(TEXT("expected_type"), TEXT("Class<UserWidget>"));
    ClassPin->SetStringField(TEXT("description"), TEXT("The widget class to instantiate"));
    ClassPin->SetBoolField(TEXT("is_required"), true);
    ClassPin->SetBoolField(TEXT("is_input"), true);
    CreateWidgetPins.Add(TEXT("Class"), ClassPin);
    
    auto OwningPlayerPin = MakeShared<FJsonObject>();
    OwningPlayerPin->SetStringField(TEXT("pin_type"), TEXT("object"));
    OwningPlayerPin->SetStringField(TEXT("expected_type"), TEXT("PlayerController"));
    OwningPlayerPin->SetStringField(TEXT("description"), TEXT("The player controller that owns this widget"));
    OwningPlayerPin->SetBoolField(TEXT("is_required"), false);
    OwningPlayerPin->SetBoolField(TEXT("is_input"), true);
    CreateWidgetPins.Add(TEXT("Owning Player"), OwningPlayerPin);
    CreateWidgetPins.Add(TEXT("OwningPlayer"), OwningPlayerPin); // Alternative name
    
    auto ReturnValuePin = MakeShared<FJsonObject>();
    ReturnValuePin->SetStringField(TEXT("pin_type"), TEXT("object"));
    ReturnValuePin->SetStringField(TEXT("expected_type"), TEXT("UserWidget"));
    ReturnValuePin->SetStringField(TEXT("description"), TEXT("The created widget instance"));
    ReturnValuePin->SetBoolField(TEXT("is_required"), false);
    ReturnValuePin->SetBoolField(TEXT("is_input"), false);
    CreateWidgetPins.Add(TEXT("Return Value"), ReturnValuePin);
    
    NodePinDatabase.Add(TEXT("Create Widget"), CreateWidgetPins);
    NodePinDatabase.Add(TEXT("CreateWidget"), CreateWidgetPins);
    
    // Get Controller node
    TMap<FString, TSharedPtr<FJsonObject>> GetControllerPins;
    
    auto TargetPin = MakeShared<FJsonObject>();
    TargetPin->SetStringField(TEXT("pin_type"), TEXT("object"));
    TargetPin->SetStringField(TEXT("expected_type"), TEXT("Pawn"));
    TargetPin->SetStringField(TEXT("description"), TEXT("The pawn to get the controller from"));
    TargetPin->SetBoolField(TEXT("is_required"), true);
    TargetPin->SetBoolField(TEXT("is_input"), true);
    GetControllerPins.Add(TEXT("Target"), TargetPin);
    
    auto ControllerReturnPin = MakeShared<FJsonObject>();
    ControllerReturnPin->SetStringField(TEXT("pin_type"), TEXT("object"));
    ControllerReturnPin->SetStringField(TEXT("expected_type"), TEXT("Controller"));
    ControllerReturnPin->SetStringField(TEXT("description"), TEXT("The controller possessing this pawn"));
    ControllerReturnPin->SetBoolField(TEXT("is_required"), false);
    ControllerReturnPin->SetBoolField(TEXT("is_input"), false);
    GetControllerPins.Add(TEXT("Return Value"), ControllerReturnPin);
    
    NodePinDatabase.Add(TEXT("Get Controller"), GetControllerPins);
    NodePinDatabase.Add(TEXT("GetController"), GetControllerPins);
    
    // Cast To nodes
    TMap<FString, TSharedPtr<FJsonObject>> CastToPins;
    
    auto ObjectPin = MakeShared<FJsonObject>();
    ObjectPin->SetStringField(TEXT("pin_type"), TEXT("object"));
    ObjectPin->SetStringField(TEXT("expected_type"), TEXT("Object"));
    ObjectPin->SetStringField(TEXT("description"), TEXT("The object to cast"));
    ObjectPin->SetBoolField(TEXT("is_required"), true);
    ObjectPin->SetBoolField(TEXT("is_input"), true);
    CastToPins.Add(TEXT("Object"), ObjectPin);
    
    auto CastSuccessPin = MakeShared<FJsonObject>();
    CastSuccessPin->SetStringField(TEXT("pin_type"), TEXT("exec"));
    CastSuccessPin->SetStringField(TEXT("expected_type"), TEXT("exec"));
    CastSuccessPin->SetStringField(TEXT("description"), TEXT("Execution path if cast succeeds"));
    CastSuccessPin->SetBoolField(TEXT("is_required"), false);
    CastSuccessPin->SetBoolField(TEXT("is_input"), false);
    CastToPins.Add(TEXT("Cast Success"), CastSuccessPin);
    
    auto CastFailPin = MakeShared<FJsonObject>();
    CastFailPin->SetStringField(TEXT("pin_type"), TEXT("exec"));
    CastFailPin->SetStringField(TEXT("expected_type"), TEXT("exec"));
    CastFailPin->SetStringField(TEXT("description"), TEXT("Execution path if cast fails"));
    CastFailPin->SetBoolField(TEXT("is_required"), false);
    CastFailPin->SetBoolField(TEXT("is_input"), false);
    CastToPins.Add(TEXT("Cast Failed"), CastFailPin);
    
    NodePinDatabase.Add(TEXT("Cast to PlayerController"), CastToPins);
    NodePinDatabase.Add(TEXT("Cast to"), CastToPins);
    
    // Look up the requested node and pin
    FString NormalizedNodeName = NodeName;
    NormalizedNodeName = NormalizedNodeName.Replace(TEXT(" "), TEXT(""));
    
    TSharedPtr<FJsonObject> PinInfo = nullptr;
    if (NodePinDatabase.Contains(NodeName))
    {
        auto& PinMap = NodePinDatabase[NodeName];
        if (PinMap.Contains(PinName))
        {
            PinInfo = PinMap[PinName];
        }
    }
    else if (NodePinDatabase.Contains(NormalizedNodeName))
    {
        auto& PinMap = NodePinDatabase[NormalizedNodeName];
        if (PinMap.Contains(PinName))
        {
            PinInfo = PinMap[PinName];
        }
    }
    
    if (PinInfo.IsValid())
    {
        ResultObj->SetBoolField(TEXT("success"), true);
        ResultObj->SetStringField(TEXT("node_name"), NodeName);
        ResultObj->SetStringField(TEXT("pin_name"), PinName);
        ResultObj->SetObjectField(TEXT("pin_info"), PinInfo);
        ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("Found pin information for '%s' on node '%s'"), *PinName, *NodeName));
    }
    else
    {
        ResultObj->SetBoolField(TEXT("success"), false);
        ResultObj->SetStringField(TEXT("node_name"), NodeName);
        ResultObj->SetStringField(TEXT("pin_name"), PinName);
        ResultObj->SetObjectField(TEXT("pin_info"), MakeShared<FJsonObject>());
        ResultObj->SetStringField(TEXT("message"), FString::Printf(TEXT("No pin information found for '%s' on node '%s'"), *PinName, *NodeName));
        
        // Provide available pins for this node if we know the node
        if (NodePinDatabase.Contains(NodeName) || NodePinDatabase.Contains(NormalizedNodeName))
        {
            TArray<TSharedPtr<FJsonValue>> AvailablePins;
            const auto& PinMap = NodePinDatabase.Contains(NodeName) ? NodePinDatabase[NodeName] : NodePinDatabase[NormalizedNodeName];
            for (const auto& Pin : PinMap)
            {
                AvailablePins.Add(MakeShared<FJsonValueString>(Pin.Key));
            }
            ResultObj->SetArrayField(TEXT("available_pins"), AvailablePins);
        }
    }
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(ResultObj.ToSharedRef(), Writer);
    
    return OutputString;
} 