#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "Dom/JsonObject.h"
#include "Commands/ComponentCreationParams.h"

/**
 * Parameters for Blueprint creation operations
 */
struct UNREALMCP_API FBlueprintCreationParams
{
    /** Name of the Blueprint to create */
    FString Name;
    
    /** Folder path where the Blueprint should be created */
    FString FolderPath;
    
    /** Parent class for the Blueprint */
    UClass* ParentClass;
    
    /** Whether to compile the Blueprint after creation */
    bool bCompileOnCreation = true;
    
    /** Default constructor */
    FBlueprintCreationParams()
        : ParentClass(nullptr)
    {
    }
    
    /**
     * Validate the parameters
     * @param OutError - Error message if validation fails
     * @return true if parameters are valid
     */
    bool IsValid(FString& OutError) const;
};

/**
 * Interface for Blueprint service operations
 * Provides abstraction for Blueprint creation, modification, and management
 */
class UNREALMCP_API IBlueprintService
{
public:
    virtual ~IBlueprintService() = default;
    
    /**
     * Create a new Blueprint
     * @param Params - Blueprint creation parameters
     * @return Created Blueprint or nullptr if failed
     */
    virtual UBlueprint* CreateBlueprint(const FBlueprintCreationParams& Params) = 0;
    
    /**
     * Add a component to an existing Blueprint
     * @param Blueprint - Target Blueprint
     * @param Params - Component creation parameters
     * @return true if component was added successfully
     */
    virtual bool AddComponentToBlueprint(UBlueprint* Blueprint, const struct FComponentCreationParams& Params) = 0;
    
    /**
     * Compile a Blueprint
     * @param Blueprint - Blueprint to compile
     * @param OutError - Error message if compilation fails
     * @return true if compilation succeeded
     */
    virtual bool CompileBlueprint(UBlueprint* Blueprint, FString& OutError) = 0;
    
    /**
     * Find a Blueprint by name
     * @param BlueprintName - Name of the Blueprint to find
     * @return Found Blueprint or nullptr
     */
    virtual UBlueprint* FindBlueprint(const FString& BlueprintName) = 0;
    
    /**
     * Add a variable to a Blueprint
     * @param Blueprint - Target Blueprint
     * @param VariableName - Name of the variable
     * @param VariableType - Type of the variable
     * @param bIsExposed - Whether the variable should be exposed to the editor
     * @return true if variable was added successfully
     */
    virtual bool AddVariableToBlueprint(UBlueprint* Blueprint, const FString& VariableName, const FString& VariableType, bool bIsExposed = false) = 0;
    
    /**
     * Set a property on a Blueprint's default object
     * @param Blueprint - Target Blueprint
     * @param PropertyName - Name of the property
     * @param PropertyValue - Value to set (as JSON)
     * @return true if property was set successfully
     */
    virtual bool SetBlueprintProperty(UBlueprint* Blueprint, const FString& PropertyName, const TSharedPtr<FJsonValue>& PropertyValue) = 0;
};