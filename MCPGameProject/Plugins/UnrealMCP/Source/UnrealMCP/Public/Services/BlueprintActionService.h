#pragma once

#include "CoreMinimal.h"
#include "Services/IBlueprintActionService.h"
#include "Dom/JsonObject.h"

/**
 * Implementation of Blueprint Action service operations
 * Provides Blueprint action discovery using FBlueprintActionDatabase
 */
class UNREALMCP_API FBlueprintActionService : public IBlueprintActionService
{
public:
    /** Constructor */
    FBlueprintActionService() = default;
    
    /** Destructor */
    virtual ~FBlueprintActionService() = default;
    
    // IBlueprintActionService interface
    virtual TSharedPtr<FJsonObject> GetActionsForPin(
        const FString& PinType,
        const FString& PinSubcategory,
        const FString& SearchFilter,
        int32 MaxResults
    ) override;

private:
    /**
     * Convert JSON string result to JSON object
     * @param JsonString - JSON string to convert
     * @return Parsed JSON object or error response
     */
    TSharedPtr<FJsonObject> ParseJsonResult(const FString& JsonString);
};