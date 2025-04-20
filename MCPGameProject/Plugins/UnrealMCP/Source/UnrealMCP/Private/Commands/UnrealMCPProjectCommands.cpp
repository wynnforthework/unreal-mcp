#include "Commands/UnrealMCPProjectCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "GameFramework/InputSettings.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "EditorAssetLibrary.h"

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
    else if (CommandType == TEXT("get_project_dir"))
    {
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetBoolField(TEXT("success"), true);
        ResultObj->SetStringField(TEXT("project_dir"), FPaths::ProjectDir());
        return ResultObj;
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