#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "UnrealMCPBlueprintActionCommands.generated.h"

/**
 * Commands for discovering Blueprint actions dynamically using the FBlueprintActionDatabase
 */
UCLASS()
class UNREALMCP_API UUnrealMCPBlueprintActionCommands : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Get all available actions for a specific pin type
     * @param PinType - The type of pin (object, int, float, bool, string, struct, etc.)
     * @param PinSubCategory - The subcategory/class name for object pins
     * @return JSON string containing available actions
     */
    UFUNCTION(BlueprintCallable, Category = "Unreal MCP Blueprint Actions")
    static FString GetActionsForPin(const FString& PinType, const FString& PinSubCategory);

    /**
     * Get all available actions for a specific class
     * @param ClassName - Name or path of the class to get actions for
     * @return JSON string containing available actions
     */
    UFUNCTION(BlueprintCallable, Category = "Unreal MCP Blueprint Actions")
    static FString GetActionsForClass(const FString& ClassName);

    /**
     * Get all available actions for a class and its entire inheritance hierarchy
     * @param ClassName - Name or path of the class to get actions for
     * @return JSON string containing available actions and hierarchy info
     */
    UFUNCTION(BlueprintCallable, Category = "Unreal MCP Blueprint Actions")
    static FString GetActionsForClassHierarchy(const FString& ClassName);

    /**
     * Get specific node pin information
     * @param NodeName - Name of the node
     * @param PinName - Name of the pin
     * @return JSON string containing pin information
     */
    UFUNCTION(BlueprintCallable, Category = "Unreal MCP Blueprint Actions")
    static FString GetNodePinInfo(const FString& NodeName, const FString& PinName);

    /**
     * Create a blueprint node by discovered action/function name
     * @param BlueprintName - Name of the target Blueprint
     * @param FunctionName - Name of the function to create a node for (from discovered actions)
     * @param ClassName - Optional class name if the function is from a specific class
     * @param NodePosition - Optional [X, Y] position for the node
     * @return JSON string containing node creation result
     */
    UFUNCTION(BlueprintCallable, Category = "Unreal MCP Blueprint Actions")
    static FString CreateNodeByActionName(const FString& BlueprintName, const FString& FunctionName, const FString& ClassName = TEXT(""), const FString& NodePosition = TEXT(""));
}; 