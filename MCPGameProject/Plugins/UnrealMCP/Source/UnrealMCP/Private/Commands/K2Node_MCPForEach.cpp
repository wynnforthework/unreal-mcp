#include "CoreMinimal.h"
#include "Commands/K2Node_MCPForEach.h"
#include "EdGraphSchema_K2.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"

#define LOCTEXT_NAMESPACE "MCPForEachNode"

UK2Node_MCPForEach::UK2Node_MCPForEach(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UK2Node_MCPForEach::AllocateDefaultPins()
{
    // Exec input
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, FName(), nullptr, UEdGraphSchema_K2::PN_Execute);
    // Exec output (Completed)
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, FName(), nullptr, UEdGraphSchema_K2::PN_Then);
    // Exec output (Loop Body)
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, FName(), nullptr, TEXT("LoopBody"));
    // Array input (wildcard)
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, FName(), nullptr, *ArrayPinName);
    // Element output (wildcard)
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, FName(), nullptr, *ElementPinName);
    // Array Index output (int)
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Int, FName(), nullptr, TEXT("Array Index"));
}

FText UK2Node_MCPForEach::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return LOCTEXT("ForEachNodeTitle", "For Each (MCP)");
}

FText UK2Node_MCPForEach::GetTooltipText() const
{
    return LOCTEXT("ForEachNodeTooltip", "Loop over each element of an array (MCP)");
}

FText UK2Node_MCPForEach::GetMenuCategory() const
{
    return LOCTEXT("ForEachNodeCategory", "MCP");
}

void UK2Node_MCPForEach::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();
    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
        check(Spawner != nullptr);
        ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
    }
}

#undef LOCTEXT_NAMESPACE 