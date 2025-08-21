#pragma once

#include "CoreMinimal.h"
#include "Services/IPropertyService.h"
#include "Dom/JsonObject.h"

/**
 * Concrete implementation of IPropertyService
 * Provides property setting and getting functionality with proper type conversion
 */
class UNREALMCP_API FPropertyService : public IPropertyService
{
public:
    /**
     * Get the singleton instance of the property service
     * @return Reference to the singleton instance
     */
    static FPropertyService& Get();
    
    // IPropertyService interface implementation
    virtual bool SetObjectProperty(UObject* Object, const FString& PropertyName, 
                                  const TSharedPtr<FJsonValue>& PropertyValue, FString& OutError) override;
    virtual bool SetObjectProperties(UObject* Object, const TSharedPtr<FJsonObject>& Properties,
                                   TArray<FString>& OutSuccessProperties,
                                   TMap<FString, FString>& OutFailedProperties) override;
    virtual bool GetObjectProperty(UObject* Object, const FString& PropertyName,
                                  TSharedPtr<FJsonValue>& OutValue, FString& OutError) override;
    virtual bool HasProperty(UObject* Object, const FString& PropertyName) override;
    virtual TArray<FString> GetObjectPropertyNames(UObject* Object) override;

private:
    /** Private constructor for singleton pattern */
    FPropertyService() = default;
    
    /**
     * Set property value from JSON with proper type conversion
     * @param Property - Property to set
     * @param PropertyData - Pointer to property data
     * @param JsonValue - JSON value to convert and set
     * @param OutError - Error message if conversion fails
     * @return true if property was set successfully
     */
    bool SetPropertyFromJson(FProperty* Property, void* PropertyData, 
                           const TSharedPtr<FJsonValue>& JsonValue, FString& OutError) const;
    
    /**
     * Get property value as JSON with proper type conversion
     * @param Property - Property to get
     * @param PropertyData - Pointer to property data
     * @param OutJsonValue - Output JSON value
     * @param OutError - Error message if conversion fails
     * @return true if property was retrieved successfully
     */
    bool GetPropertyAsJson(FProperty* Property, const void* PropertyData,
                          TSharedPtr<FJsonValue>& OutJsonValue, FString& OutError) const;
    
    /**
     * Handle special collision properties that need custom logic
     * @param Object - Target object (must be UPrimitiveComponent)
     * @param PropertyName - Name of the collision property
     * @param PropertyValue - JSON value to set
     * @return true if property was handled and set
     */
    bool HandleCollisionProperty(UObject* Object, const FString& PropertyName,
                               const TSharedPtr<FJsonValue>& PropertyValue) const;
};
