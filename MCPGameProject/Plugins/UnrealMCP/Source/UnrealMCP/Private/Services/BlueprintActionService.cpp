#include "Services/BlueprintActionService.h"
#include "Commands/UnrealMCPBlueprintActionCommands.h"

FString FBlueprintActionService::GetActionsForPin(const FString& PinType, const FString& PinSubCategory, const FString& SearchFilter, int32 MaxResults)
{
    return UUnrealMCPBlueprintActionCommands::GetActionsForPin(PinType, PinSubCategory, SearchFilter, MaxResults);
}

FString FBlueprintActionService::GetActionsForClass(const FString& ClassName, const FString& SearchFilter, int32 MaxResults)
{
    return UUnrealMCPBlueprintActionCommands::GetActionsForClass(ClassName, SearchFilter, MaxResults);
}

FString FBlueprintActionService::GetActionsForClassHierarchy(const FString& ClassName, const FString& SearchFilter, int32 MaxResults)
{
    return UUnrealMCPBlueprintActionCommands::GetActionsForClassHierarchy(ClassName, SearchFilter, MaxResults);
}

FString FBlueprintActionService::SearchBlueprintActions(const FString& SearchQuery, const FString& Category, int32 MaxResults, const FString& BlueprintName)
{
    return UUnrealMCPBlueprintActionCommands::SearchBlueprintActions(SearchQuery, Category, MaxResults, BlueprintName);
}

FString FBlueprintActionService::GetNodePinInfo(const FString& NodeName, const FString& PinName)
{
    return UUnrealMCPBlueprintActionCommands::GetNodePinInfo(NodeName, PinName);
}

FString FBlueprintActionService::CreateNodeByActionName(const FString& BlueprintName, const FString& FunctionName, const FString& ClassName, const FString& NodePosition, const FString& JsonParams)
{
    return UUnrealMCPBlueprintActionCommands::CreateNodeByActionName(BlueprintName, FunctionName, ClassName, NodePosition, JsonParams);
}