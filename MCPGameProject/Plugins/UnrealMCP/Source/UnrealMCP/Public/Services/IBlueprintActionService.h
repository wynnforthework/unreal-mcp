#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Interface for Blueprint Action service operations
 * Provides abstraction for Blueprint action discovery using FBlueprintActionDatabase
 */
class UNREALMCP_API IBlueprintActionService
{
public:
    virtual ~IBlueprintActionService() = default;
    
    /**
     * Get all available Blueprint actions for a specific pin type
     * @param PinType - Pin type to search for (object, float, int, bool, string, struct, etc.)
     * @param PinSubcategory - Pin subcategory/class name for object pins (e.g., "PlayerController", "Pawn")
     * @param SearchFilter - Search filter to apply to results
     * @param MaxResults - Maximum number of results to return
     * @return JSON object containing discovered actions
     */
    virtual TSharedPtr<FJsonObject> GetActionsForPin(
        const FString& PinType,
        const FString& PinSubcategory,
        const FString& SearchFilter,
        int32 MaxResults
    ) = 0;
};