#include "Commands/UnrealMCPProjectCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
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

FUnrealMCPProjectCommands::FUnrealMCPProjectCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_input_mapping"))
    {
        return HandleCreateInputMapping(Params);
    }
    else if (CommandType == TEXT("create_folder"))
    {
        return HandleCreateFolder(Params);
    }
    else if (CommandType == TEXT("create_struct"))
    {
        return HandleCreateStruct(Params);
    }
    else if (CommandType == TEXT("update_struct"))
    {
        return HandleUpdateStruct(Params);
    }
    else if (CommandType == TEXT("get_project_dir"))
    {
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetBoolField(TEXT("success"), true);
        ResultObj->SetStringField(TEXT("project_dir"), FPaths::ProjectDir());
        return ResultObj;
    }
    else if (CommandType == TEXT("show_struct_variables"))
    {
        return HandleShowStructVariables(Params);
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown project command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleCreateInputMapping(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString ActionName;
    if (!Params->TryGetStringField(TEXT("action_name"), ActionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'action_name' parameter"));
    }

    FString Key;
    if (!Params->TryGetStringField(TEXT("key"), Key))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'key' parameter"));
    }

    // Get the input settings
    UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
    if (!InputSettings)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get input settings"));
    }

    // Create the input action mapping
    FInputActionKeyMapping ActionMapping;
    ActionMapping.ActionName = FName(*ActionName);
    ActionMapping.Key = FKey(*Key);

    // Add modifiers if provided
    if (Params->HasField(TEXT("shift")))
    {
        ActionMapping.bShift = Params->GetBoolField(TEXT("shift"));
    }
    if (Params->HasField(TEXT("ctrl")))
    {
        ActionMapping.bCtrl = Params->GetBoolField(TEXT("ctrl"));
    }
    if (Params->HasField(TEXT("alt")))
    {
        ActionMapping.bAlt = Params->GetBoolField(TEXT("alt"));
    }
    if (Params->HasField(TEXT("cmd")))
    {
        ActionMapping.bCmd = Params->GetBoolField(TEXT("cmd"));
    }

    // Add the mapping
    InputSettings->AddActionMapping(ActionMapping);
    InputSettings->SaveConfig();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("action_name"), ActionName);
    ResultObj->SetStringField(TEXT("key"), Key);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleCreateFolder(const TSharedPtr<FJsonObject>& Params)
{
    FString FolderPath;
    if (!Params->TryGetStringField(TEXT("folder_path"), FolderPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'folder_path' parameter"));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    // Get the base project directory
    FString ProjectPath = FPaths::ProjectDir();
    
    // Check if this is a content folder request
    bool bIsContentFolder = FolderPath.StartsWith(TEXT("/Content/")) || FolderPath.StartsWith(TEXT("Content/"));
    if (bIsContentFolder)
    {
        // Use UE's asset system for content folders
        FString AssetPath = FolderPath;
        if (!AssetPath.StartsWith(TEXT("/Game/")))
        {
            // Convert Content/ to /Game/ for asset paths
            AssetPath = AssetPath.Replace(TEXT("/Content/"), TEXT("/Game/"));
            AssetPath = AssetPath.Replace(TEXT("Content/"), TEXT("/Game/"));
        }
        
        // Check if the directory already exists
        if (UEditorAssetLibrary::DoesDirectoryExist(AssetPath))
        {
            ResultObj->SetBoolField(TEXT("success"), true);
            ResultObj->SetStringField(TEXT("folder_path"), FolderPath);
            ResultObj->SetBoolField(TEXT("already_exists"), true);
            return ResultObj;
        }
        
        if (!UEditorAssetLibrary::MakeDirectory(AssetPath))
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create content folder: %s"), *AssetPath));
        }
    }
    else
    {
        // For non-content folders, use platform file system
        FString FullPath = FPaths::Combine(ProjectPath, FolderPath);
        
        // Check if directory already exists
        if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FullPath))
        {
            ResultObj->SetBoolField(TEXT("success"), true);
            ResultObj->SetStringField(TEXT("folder_path"), FolderPath);
            ResultObj->SetBoolField(TEXT("already_exists"), true);
            return ResultObj;
        }
        
        if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*FullPath))
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create folder: %s"), *FullPath));
        }
    }

    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("folder_path"), FolderPath);
    ResultObj->SetBoolField(TEXT("already_exists"), false);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleCreateStruct(const TSharedPtr<FJsonObject>& Params)
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
    FString Path = TEXT("/Game/Blueprints");
    Params->TryGetStringField(TEXT("path"), Path);

    FString Description = TEXT("");
    Params->TryGetStringField(TEXT("description"), Description);

    // Make sure the path exists
    if (!UEditorAssetLibrary::DoesDirectoryExist(Path))
    {
        if (!UEditorAssetLibrary::MakeDirectory(Path))
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create directory: %s"), *Path));
        }
    }

    // Create the struct asset path
    FString AssetName = StructName;
    FString PackagePath = Path;
    if (!PackagePath.EndsWith(TEXT("/")))
    {
        PackagePath += TEXT("/");
    }
    FString PackageName = PackagePath + AssetName;

    // Check if the struct already exists
    if (UEditorAssetLibrary::DoesAssetExist(PackageName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Struct already exists: %s"), *PackageName));
    }

    // Use AssetTools and UStructureFactory to create the struct asset
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    UStructureFactory* StructFactory = NewObject<UStructureFactory>();
    UObject* CreatedAsset = AssetToolsModule.Get().CreateAsset(AssetName, PackagePath.LeftChop(1), UUserDefinedStruct::StaticClass(), StructFactory);
    UUserDefinedStruct* NewStruct = Cast<UUserDefinedStruct>(CreatedAsset);
    
    if (!NewStruct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create struct asset"));
    }

    // Set the struct description and tooltip
    if (!Description.IsEmpty())
    {
        NewStruct->SetMetaData(TEXT("Comments"), *Description);
        FStructureEditorUtils::ChangeTooltip(NewStruct, Description);
    }

    // Remove all existing variables from the struct before adding new ones
    {
        auto VarDescArray = FStructureEditorUtils::GetVarDesc(NewStruct);
        TArray<FGuid> GuidsToRemove;
        for (const FStructVariableDescription& Desc : VarDescArray)
        {
            GuidsToRemove.Add(Desc.VarGuid);
        }
        for (const FGuid& Guid : GuidsToRemove)
        {
            FStructureEditorUtils::RemoveVariable(NewStruct, Guid);
        }
    }
    
    // Keep track of the variables we've attempted to add
    TArray<FString> AddedProperties;
    TSet<FGuid> RenamedGuids; // Track GUIDs we've already renamed
    
    // Add simple variables - this is a simplified approach that should work with UE 5.5
    int32 PropertyIndex = 0;
    for (const TSharedPtr<FJsonValue>& PropertyValue : *PropertiesArray)
    {
        TSharedPtr<FJsonObject> PropertyObj = PropertyValue->AsObject();
        if (!PropertyObj.IsValid())
        {
            ++PropertyIndex;
            continue;
        }

        FString PropertyName;
        if (!PropertyObj->TryGetStringField(TEXT("name"), PropertyName))
        {
            ++PropertyIndex;
            continue;
        }

        // Skip duplicates
        if (AddedProperties.Contains(PropertyName))
        {
            ++PropertyIndex;
            continue;
        }
        
        AddedProperties.Add(PropertyName);

        // Set up the property type
        FString PropertyType;
        if (!PropertyObj->TryGetStringField(TEXT("type"), PropertyType))
        {
            ++PropertyIndex;
            continue;
        }

        FEdGraphPinType PinType;
        if (PropertyType.Equals(TEXT("Boolean"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("bool");
        }
        else if (PropertyType.Equals(TEXT("Integer"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("int");
        }
        else if (PropertyType.Equals(TEXT("Float"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("float");
        }
        else if (PropertyType.Equals(TEXT("String"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("string");
        }
        else if (PropertyType.Equals(TEXT("Name"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("name");
        }
        else if (PropertyType.Equals(TEXT("Vector"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("struct");
            PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
        }
        else if (PropertyType.Equals(TEXT("Rotator"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("struct");
            PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
        }
        else if (PropertyType.Equals(TEXT("Transform"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("struct");
            PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
        }
        else if (PropertyType.Equals(TEXT("Color"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = TEXT("struct");
            PinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
        }
        else
        {
            // Default to string if type not recognized
            PinType.PinCategory = TEXT("string");
        }

        // Add the variable
        FStructureEditorUtils::AddVariable(NewStruct, PinType);

        // Find and rename the just-added variable
        bool bRenamed = false;
        const auto& VarDescArray = FStructureEditorUtils::GetVarDesc(NewStruct);
        for (const FStructVariableDescription& Desc : VarDescArray)
        {
            if (Desc.VarName.ToString().StartsWith(TEXT("MemberVar_")) && !RenamedGuids.Contains(Desc.VarGuid))
            {
                // Rename this variable
                FStructureEditorUtils::RenameVariable(NewStruct, Desc.VarGuid, PropertyName);
                RenamedGuids.Add(Desc.VarGuid);
                bRenamed = true;

                // Set variable tooltip if provided
                FString VarTooltip;
                if (PropertyObj->TryGetStringField(TEXT("description"), VarTooltip) || PropertyObj->TryGetStringField(TEXT("tooltip"), VarTooltip))
                {
                    FStructureEditorUtils::ChangeVariableTooltip(NewStruct, Desc.VarGuid, VarTooltip);
                }
                break;
            }
        }

        // If we failed to rename the variable, remove it to avoid leaving unrenamed variables
        if (!bRenamed)
        {
            // Find and remove the unrenamed variable
            for (const FStructVariableDescription& Desc : VarDescArray)
            {
                if (Desc.VarName.ToString().StartsWith(TEXT("MemberVar_")) && !RenamedGuids.Contains(Desc.VarGuid))
                {
                    FStructureEditorUtils::RemoveVariable(NewStruct, Desc.VarGuid);
                    break;
                }
            }
            AddedProperties.Remove(PropertyName); // Remove from our tracking since it failed
        }

        ++PropertyIndex;
    }

    // Clean up any remaining unrenamed variables
    const auto& FinalVarDescArray = FStructureEditorUtils::GetVarDesc(NewStruct);
    for (const FStructVariableDescription& Desc : FinalVarDescArray)
    {
        if (Desc.VarName.ToString().StartsWith(TEXT("MemberVar_")) && !RenamedGuids.Contains(Desc.VarGuid))
        {
            FStructureEditorUtils::RemoveVariable(NewStruct, Desc.VarGuid);
        }
    }

    // Final compilation
    FStructureEditorUtils::CompileStructure(NewStruct);
    
    // Save the struct
    NewStruct->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(NewStruct);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("struct_name"), StructName);
    ResultObj->SetStringField(TEXT("path"), Path);
    ResultObj->SetStringField(TEXT("full_path"), PackageName);
    
    return ResultObj;
}

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
    FString Path = TEXT("/Game/Blueprints");
    Params->TryGetStringField(TEXT("path"), Path);

    FString Description = TEXT("");
    Params->TryGetStringField(TEXT("description"), Description);

    // Create the struct asset path
    FString AssetName = StructName;
    FString PackagePath = Path;
    if (!PackagePath.EndsWith(TEXT("/")))
    {
        PackagePath += TEXT("/");
    }
    FString PackageName = PackagePath + AssetName;

    // Check if the struct exists
    if (!UEditorAssetLibrary::DoesAssetExist(PackageName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Struct does not exist: %s"), *PackageName));
    }

    UObject* AssetObj = UEditorAssetLibrary::LoadAsset(PackageName);
    UUserDefinedStruct* ExistingStruct = Cast<UUserDefinedStruct>(AssetObj);
    if (!ExistingStruct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to load struct asset"));
    }

    // Set the struct description and tooltip
    if (!Description.IsEmpty())
    {
        ExistingStruct->SetMetaData(TEXT("Comments"), *Description);
        FStructureEditorUtils::ChangeTooltip(ExistingStruct, Description);
    }

    // Build a map of existing variables by name
    TMap<FString, FStructVariableDescription> ExistingVarsByName;
    auto InitialVarDescArray = FStructureEditorUtils::GetVarDesc(ExistingStruct);
    UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Initial variables in struct:"));
    for (const FStructVariableDescription& Desc : InitialVarDescArray) {
        UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Desc.VarName.ToString());
    }
    for (const FStructVariableDescription& Desc : InitialVarDescArray)
    {
        ExistingVarsByName.Add(Desc.VarName.ToString(), Desc);
    }

    // Track which variables were updated or added
    TSet<FString> UpdatedOrAddedNames;

    int32 PropertyIndex = 0;
    for (const TSharedPtr<FJsonValue>& PropertyValue : *PropertiesArray)
    {
        TSharedPtr<FJsonObject> PropertyObj = PropertyValue->AsObject();
        if (!PropertyObj.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Property at index %d is invalid, skipping."), PropertyIndex);
            ++PropertyIndex;
            continue;
        }

        FString PropertyName;
        if (!PropertyObj->TryGetStringField(TEXT("name"), PropertyName))
        {
            UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Property at index %d missing 'name', skipping."), PropertyIndex);
            ++PropertyIndex;
            continue;
        }

        FString PropertyType;
        if (!PropertyObj->TryGetStringField(TEXT("type"), PropertyType))
        {
            UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Property '%s' missing 'type', skipping."), *PropertyName);
            ++PropertyIndex;
            continue;
        }

        FString PropertyTooltip;
        PropertyObj->TryGetStringField(TEXT("description"), PropertyTooltip);

        if (ExistingVarsByName.Contains(PropertyName))
        {
            UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Property '%s' already exists, will update tooltip if needed."), *PropertyName);
            // Update tooltip if needed (type change is not supported by UE, so log if mismatch)
            const FStructVariableDescription& ExistingDesc = ExistingVarsByName[PropertyName];
            // There is no Type member in FStructVariableDescription in UE 5.5, so we cannot compare types directly.
            // Instead, just update the tooltip if needed and log that type changes are not supported.
            if (!PropertyTooltip.IsEmpty() && PropertyTooltip != ExistingDesc.ToolTip)
            {
                FStructureEditorUtils::ChangeVariableTooltip(ExistingStruct, ExistingDesc.VarGuid, PropertyTooltip);
                UE_LOG(LogTemp, Display, TEXT("[MCP UpdateStruct] Updated tooltip for '%s' to '%s'."), *PropertyName, *PropertyTooltip);
            }
            // Log that type changes are not supported
            UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Type change for '%s' is not supported and will be ignored (cannot check type in UE 5.5)."), *PropertyName);
            UpdatedOrAddedNames.Add(PropertyName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Property '%s' does not exist, will add as new variable."), *PropertyName);
            // Add new variable
            FEdGraphPinType PinType;
            if (PropertyType.Equals(TEXT("Boolean"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("bool");
            }
            else if (PropertyType.Equals(TEXT("Integer"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("int");
            }
            else if (PropertyType.Equals(TEXT("Float"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("float");
            }
            else if (PropertyType.Equals(TEXT("String"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("string");
            }
            else if (PropertyType.Equals(TEXT("Name"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("name");
            }
            else if (PropertyType.Equals(TEXT("Vector"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("struct");
                PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
            }
            else if (PropertyType.Equals(TEXT("Rotator"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("struct");
                PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
            }
            else if (PropertyType.Equals(TEXT("Transform"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("struct");
                PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
            }
            else if (PropertyType.Equals(TEXT("Color"), ESearchCase::IgnoreCase))
            {
                PinType.PinCategory = TEXT("struct");
                PinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
            }
            else
            {
                PinType.PinCategory = TEXT("string");
            }
            FStructureEditorUtils::AddVariable(ExistingStruct, PinType);
            // Find and rename the just-added variable
            auto VarDescArray = FStructureEditorUtils::GetVarDesc(ExistingStruct);
            for (const FStructVariableDescription& Desc : VarDescArray)
            {
                if (Desc.VarName.ToString().StartsWith(TEXT("MemberVar_")))
                {
                    UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Renaming new variable %s to %s"), *Desc.VarName.ToString(), *PropertyName);
                    FStructureEditorUtils::RenameVariable(ExistingStruct, Desc.VarGuid, PropertyName);
                    if (!PropertyTooltip.IsEmpty())
                    {
                        FStructureEditorUtils::ChangeVariableTooltip(ExistingStruct, Desc.VarGuid, PropertyTooltip);
                    }
                    UE_LOG(LogTemp, Display, TEXT("[MCP UpdateStruct] Added new variable '%s' (type: %s)."), *PropertyName, *PropertyType);
                    break;
                }
            }
            UpdatedOrAddedNames.Add(PropertyName);
        }
        ++PropertyIndex;
    }

    // Final compilation
    FStructureEditorUtils::CompileStructure(ExistingStruct);
    ExistingStruct->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(ExistingStruct);
    UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Variables in struct after update, before cleanup:"));
    auto PreCleanupVarDescArray = FStructureEditorUtils::GetVarDesc(ExistingStruct);
    for (const FStructVariableDescription& Desc : PreCleanupVarDescArray) {
        UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Desc.VarName.ToString());
    }
    // Clean up any remaining unrenamed variables (MemberVar_)
    TArray<FGuid> GuidsToRemove;
    for (const FStructVariableDescription& Desc : FStructureEditorUtils::GetVarDesc(ExistingStruct))
    {
        if (Desc.VarName.ToString().StartsWith(TEXT("MemberVar_")))
        {
            UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Will remove leftover variable: %s"), *Desc.VarName.ToString());
            GuidsToRemove.Add(Desc.VarGuid);
        }
    }
    for (const FGuid& Guid : GuidsToRemove)
    {
        FStructureEditorUtils::RemoveVariable(ExistingStruct, Guid);
        UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Removed variable with GUID: %s"), *Guid.ToString());
    }
    UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Cleanup removed %d variables."), GuidsToRemove.Num());
    // Log final state
    auto FinalVarDescArray = FStructureEditorUtils::GetVarDesc(ExistingStruct);
    UE_LOG(LogTemp, Warning, TEXT("[MCP UpdateStruct] Final variables in struct after cleanup:"));
    for (const FStructVariableDescription& Desc : FinalVarDescArray) {
        UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Desc.VarName.ToString());
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("struct_name"), StructName);
    ResultObj->SetStringField(TEXT("path"), Path);
    ResultObj->SetStringField(TEXT("full_path"), PackageName);
    
    return ResultObj;
}

// Helper to get a user-friendly type string for a property
static FString GetPropertyTypeString(const FProperty* Property)
{
    if (Property->IsA<FBoolProperty>()) return TEXT("Boolean");
    if (Property->IsA<FIntProperty>()) return TEXT("Integer");
    if (Property->IsA<FFloatProperty>()) return TEXT("Float");
    if (Property->IsA<FStrProperty>()) return TEXT("String");
    if (Property->IsA<FNameProperty>()) return TEXT("Name");
    if (Property->IsA<FStructProperty>())
    {
        const FStructProperty* StructProp = CastField<FStructProperty>(Property);
        if (StructProp->Struct == TBaseStructure<FVector>::Get()) return TEXT("Vector");
        if (StructProp->Struct == TBaseStructure<FRotator>::Get()) return TEXT("Rotator");
        if (StructProp->Struct == TBaseStructure<FTransform>::Get()) return TEXT("Transform");
        if (StructProp->Struct == TBaseStructure<FLinearColor>::Get()) return TEXT("Color");
        return StructProp->Struct->GetName();
    }
    if (Property->IsA<FArrayProperty>()) return TEXT("Array");
    // Add more types as needed
    return Property->GetClass()->GetName();
}

TSharedPtr<FJsonObject> FUnrealMCPProjectCommands::HandleShowStructVariables(const TSharedPtr<FJsonObject>& Params)
{
    FString StructName;
    if (!Params->TryGetStringField(TEXT("struct_name"), StructName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_name' parameter"));
    }

    FString Path = TEXT("/Game/Blueprints");
    Params->TryGetStringField(TEXT("path"), Path);

    FString AssetName = StructName;
    FString PackagePath = Path;
    if (!PackagePath.EndsWith(TEXT("/")))
    {
        PackagePath += TEXT("/");
    }
    FString PackageName = PackagePath + AssetName;

    if (!UEditorAssetLibrary::DoesAssetExist(PackageName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Struct does not exist: %s"), *PackageName));
    }

    UObject* AssetObj = UEditorAssetLibrary::LoadAsset(PackageName);
    UUserDefinedStruct* Struct = Cast<UUserDefinedStruct>(AssetObj);
    if (!Struct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to load struct asset"));
    }

    auto VarDescArray = FStructureEditorUtils::GetVarDesc(Struct);
    TArray<TSharedPtr<FJsonValue>> VariablesJson;
    for (const FStructVariableDescription& Desc : VarDescArray)
    {
        TSharedPtr<FJsonObject> VarObj = MakeShared<FJsonObject>();
        VarObj->SetStringField(TEXT("name"), Desc.VarName.ToString());
        if (FProperty* Property = Struct->FindPropertyByName(Desc.VarName))
        {
            FString TypeString = GetPropertyTypeString(Property);
            VarObj->SetStringField(TEXT("type"), TypeString);
        }
        else
        {
            VarObj->SetStringField(TEXT("type"), TEXT("Unknown"));
        }
        VarObj->SetStringField(TEXT("tooltip"), Desc.ToolTip);
        VariablesJson.Add(MakeShared<FJsonValueObject>(VarObj));
    }

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetStringField(TEXT("struct_name"), StructName);
    ResultObj->SetArrayField(TEXT("variables"), VariablesJson);
    return ResultObj;
}
