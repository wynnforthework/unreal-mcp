#include "Commands/UnrealMCPProjectCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Commands/UnrealMCPCommandRegistry.h"
#include "Commands/ProjectCommandRegistration.h"
#include "Services/IProjectService.h"
#include "Services/ProjectService.h"
#include "GameFramework/InputSettings.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "EditorAssetLibrary.h"
#include "Engine/UserDefinedStruct.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/StructureEditorUtils.h"
#include "UnrealEd.h"
#include "AssetToolsModule.h"
#include "Factories/StructureFactory.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "EnhancedInput/Public/InputAction.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "Factories/Factory.h"
#include "Misc/PackageName.h"

// Helper to get a user-friendly type string for a property
static FString GetPropertyTypeString(const FProperty* Property)
{
    if (!Property) return TEXT("Unknown");
    
    // Handle array properties first
    if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
    {
        // Get the type of array elements
        FString ElementType = GetPropertyTypeString(ArrayProp->Inner);
        return ElementType + TEXT("[]");
    }
    
    if (Property->IsA<FBoolProperty>()) return TEXT("Boolean");
    if (Property->IsA<FIntProperty>()) return TEXT("Integer");
    if (Property->IsA<FFloatProperty>() || Property->IsA<FDoubleProperty>()) return TEXT("Float");
    if (Property->IsA<FStrProperty>()) return TEXT("String");
    if (Property->IsA<FNameProperty>()) return TEXT("Name");
    if (const FStructProperty* StructProp = CastField<FStructProperty>(Property))
    {
        if (StructProp->Struct == TBaseStructure<FVector>::Get()) return TEXT("Vector");
        if (StructProp->Struct == TBaseStructure<FRotator>::Get()) return TEXT("Rotator");
        if (StructProp->Struct == TBaseStructure<FTransform>::Get()) return TEXT("Transform");
        if (StructProp->Struct == TBaseStructure<FLinearColor>::Get()) return TEXT("Color");
        // For custom structs, strip the 'F' prefix if present
        FString StructName = StructProp->Struct->GetName();
        if (StructName.StartsWith(TEXT("F")) && StructName.Len() > 1)
        {
            StructName = StructName.RightChop(1);
        }
        return StructName;
    }
    return TEXT("Unknown");
}

FUnrealMCPProjectCommands::FUnrealMCPProjectCommands()
{
    InitializeServices();
    RegisterCommands();
}

FUnrealMCPProjectCommands::~FUnrealMCPProjectCommands()
{
}

void FUnrealMCPProjectCommands::InitializeServices()
{
    // Create the project service
    ProjectService = MakeShared<FProjectService>();
    
    // Use the singleton command registry
    // Note: We'll store a reference to the singleton, but we don't own it
}

void FUnrealMCPProjectCommands::RegisterCommands()
{
    if (ProjectService.IsValid())
    {
        FProjectCommandRegistration::RegisterCommands(FUnrealMCPCommandRegistry::Get(), ProjectService);
    }
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    // Try to handle with the new command registry first
    FUnrealMCPCommandRegistry& Registry = FUnrealMCPCommandRegistry::Get();
    if (Registry.IsCommandRegistered(CommandType))
    {
        // Convert TSharedPtr<FJsonObject> to JSON string
        FString JsonString;
        if (Params.IsValid())
        {
            TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
            FJsonSerializer::Serialize(Params.ToSharedRef(), Writer);
        }
        
        // Execute command and get JSON string response
        FString ResponseString = Registry.ExecuteCommand(CommandType, JsonString);
        
        // Parse response back to TSharedPtr<FJsonObject>
        TSharedPtr<FJsonObject> ResponseObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);
        if (FJsonSerializer::Deserialize(Reader, ResponseObject))
        {
            return ResponseObject;
        }
        else
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to parse command response"));
        }
    }
    
    // Fall back to legacy handlers for commands not yet migrated
    return HandleLegacyCommand(CommandType, Params);
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleLegacyCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("update_struct"))
    {
        return HandleUpdateStruct(Params);
    }
    else if (CommandType == TEXT("show_struct_variables"))
    {
        return HandleShowStructVariables(Params);
    }
    else if (CommandType == TEXT("list_folder_contents"))
    {
        return HandleListFolderContents(Params);
    }
    else if (CommandType == TEXT("create_enhanced_input_action"))
    {
        return HandleCreateEnhancedInputAction(Params);
    }
    else if (CommandType == TEXT("create_input_mapping_context"))
    {
        return HandleCreateInputMappingContext(Params);
    }
    else if (CommandType == TEXT("add_mapping_to_context"))
    {
        return HandleAddMappingToContext(Params);
    }
    else if (CommandType == TEXT("list_input_actions"))
    {
        return HandleListInputActions(Params);
    }
    else if (CommandType == TEXT("list_input_mapping_contexts"))
    {
        return HandleListInputMappingContexts(Params);
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown project command: %s"), *CommandType));
}

// Legacy command implementations - these will be migrated to individual command classes later

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleUpdateStruct(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString StructName;
    if (!Params->TryGetStringField(TEXT("struct_name"), StructName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_name' parameter"));
    }

    const TArray<TSharedPtr<FJsonValue>>* PropertiesArray;
    if (!Params->TryGetArrayField(TEXT("properties"), PropertiesArray))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'properties' parameter"));
    }

    // Get optional parameters
    FString Path = TEXT("/Game");
    Params->TryGetStringField(TEXT("path"), Path);

    FString Description;
    Params->TryGetStringField(TEXT("description"), Description);

    // Convert properties array to the format expected by the service
    TArray<TSharedPtr<FJsonObject>> Properties;
    for (const TSharedPtr<FJsonValue>& PropertyValue : *PropertiesArray)
    {
        if (TSharedPtr<FJsonObject> PropertyObj = PropertyValue->AsObject())
        {
            Properties.Add(PropertyObj);
        }
    }

    // Execute the operation using the service
    FString Error;
    if (!ProjectService->UpdateStruct(StructName, Path, Description, Properties, Error))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
    }

    // Create success response
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("struct_name"), StructName);
    ResultObj->SetStringField(TEXT("path"), Path);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleShowStructVariables(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString StructName;
    if (!Params->TryGetStringField(TEXT("struct_name"), StructName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_name' parameter"));
    }

    // Get optional parameters
    FString Path = TEXT("/Game");
    Params->TryGetStringField(TEXT("path"), Path);

    // Execute the operation using the service
    bool bSuccess = false;
    FString Error;
    TArray<TSharedPtr<FJsonObject>> Variables = ProjectService->ShowStructVariables(StructName, Path, bSuccess, Error);
    
    if (!bSuccess)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
    }

    // Create success response
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("struct_name"), StructName);
    
    // Convert variables to JSON array
    TArray<TSharedPtr<FJsonValue>> VariableArray;
    for (const TSharedPtr<FJsonObject>& Variable : Variables)
    {
        VariableArray.Add(MakeShared<FJsonValueObject>(Variable));
    }
    ResultObj->SetArrayField(TEXT("variables"), VariableArray);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleListFolderContents(const TSharedPtr<FJsonObject>& Params)
{
    FString FolderPath;
    if (!Params->TryGetStringField(TEXT("folder_path"), FolderPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'folder_path' parameter"));
    }

    // Execute the operation using the service
    bool bSuccess = false;
    FString Error;
    TArray<FString> Contents = ProjectService->ListFolderContents(FolderPath, bSuccess, Error);
    
    if (!bSuccess)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
    }

    // Create success response
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("folder_path"), FolderPath);
    
    // Convert contents to JSON array
    TArray<TSharedPtr<FJsonValue>> ContentsArray;
    for (const FString& Content : Contents)
    {
        ContentsArray.Add(MakeShared<FJsonValueString>(Content));
    }
    ResultObj->SetArrayField(TEXT("contents"), ContentsArray);
    
    return ResultObj;
}

// Placeholder implementations for Enhanced Input commands - these need full implementation
TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleCreateEnhancedInputAction(const TSharedPtr<FJsonObject>& Params)
{
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Enhanced Input Action creation not yet fully implemented"));
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleCreateInputMappingContext(const TSharedPtr<FJsonObject>& Params)
{
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Input Mapping Context creation not yet fully implemented"));
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleAddMappingToContext(const TSharedPtr<FJsonObject>& Params)
{
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Add mapping to context not yet fully implemented"));
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleListInputActions(const TSharedPtr<FJsonObject>& Params)
{
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("List input actions not yet fully implemented"));
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleListInputMappingContexts(const TSharedPtr<FJsonObject>& Params)
{
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("List input mapping contexts not yet fully implemented"));
}