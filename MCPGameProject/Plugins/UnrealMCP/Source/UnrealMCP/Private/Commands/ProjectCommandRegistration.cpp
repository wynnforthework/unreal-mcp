#include "Commands/ProjectCommandRegistration.h"
#include "Commands/UnrealMCPCommandRegistry.h"
#include "Commands/Project/CreateInputMappingCommand.h"
#include "Commands/Project/CreateFolderCommand.h"
#include "Commands/Project/CreateStructCommand.h"
#include "Commands/Project/GetProjectDirCommand.h"
#include "Services/IProjectService.h"

void FProjectCommandRegistration::RegisterCommands(FUnrealMCPCommandRegistry& Registry, TSharedPtr<IProjectService> ProjectService)
{
    if (!ProjectService.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ProjectService is null, cannot register project commands"));
        return;
    }

    // Register input mapping command
    Registry.RegisterCommand(MakeShared<FCreateInputMappingCommand>(ProjectService));
    
    // Register folder command
    Registry.RegisterCommand(MakeShared<FCreateFolderCommand>(ProjectService));
    
    // Register struct command
    Registry.RegisterCommand(MakeShared<FCreateStructCommand>(ProjectService));
    
    // Register get project directory command
    Registry.RegisterCommand(MakeShared<FGetProjectDirCommand>(ProjectService));
    
    // Note: Additional project commands (update_struct, show_struct_variables) 
    // are handled by legacy command system and will be migrated in future iterations
    // - list_folder_contents
    // - create_enhanced_input_action
    // - create_input_mapping_context
    // - add_mapping_to_context
    // - list_input_actions
    // - list_input_mapping_contexts

    UE_LOG(LogTemp, Log, TEXT("Registered project commands successfully"));
}