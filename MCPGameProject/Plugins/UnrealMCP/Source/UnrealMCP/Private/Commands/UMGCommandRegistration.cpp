#include "Commands/UMGCommandRegistration.h"
#include "Commands/UnrealMCPCommandRegistry.h"
#include "Commands/UMG/CreateWidgetBlueprintCommand.h"
#include "Commands/UMG/BindWidgetEventCommand.h"
#include "Commands/UMG/SetTextBlockBindingCommand.h"
#include "Commands/UMG/AddWidgetComponentCommand.h"
#include "Commands/UMG/SetWidgetPropertyCommand.h"
#include "Commands/UMG/AddChildWidgetCommand.h"
#include "Commands/UMG/CreateParentChildWidgetCommand.h"
#include "Commands/UMG/CheckWidgetComponentExistsCommand.h"
#include "Commands/UMG/SetWidgetPlacementCommand.h"
#include "Commands/UMG/GetWidgetContainerDimensionsCommand.h"
#include "Commands/UMG/GetWidgetComponentLayoutCommand.h"
#include "Services/UMG/UMGService.h"

// Static member definition
TArray<FString> FUMGCommandRegistration::RegisteredCommandNames;

void FUMGCommandRegistration::RegisterAllUMGCommands()
{
    UE_LOG(LogTemp, Log, TEXT("FUMGCommandRegistration::RegisterAllUMGCommands: Starting UMG command registration"));
    
    // Clear any existing registrations
    RegisteredCommandNames.Empty();
    
    // Register existing implemented commands
    RegisterCreateWidgetBlueprintCommand();
    RegisterBindWidgetEventCommand();
    RegisterSetTextBlockBindingCommand();
    RegisterAddWidgetComponentCommand();
    RegisterSetWidgetPropertyCommand();
    RegisterAddChildWidgetCommand();
    RegisterCreateParentChildWidgetCommand();
    RegisterCheckWidgetComponentExistsCommand();
    RegisterSetWidgetPlacementCommand();
    RegisterGetWidgetContainerDimensionsCommand();
    RegisterGetWidgetComponentLayoutCommand();
    
    // TODO: Register remaining 22 UMG commands when their classes are implemented
    // For now, we'll register the core commands that exist
    
    UE_LOG(LogTemp, Log, TEXT("FUMGCommandRegistration::RegisterAllUMGCommands: Registered %d UMG commands"), 
        RegisteredCommandNames.Num());
}

void FUMGCommandRegistration::UnregisterAllUMGCommands()
{
    UE_LOG(LogTemp, Log, TEXT("FUMGCommandRegistration::UnregisterAllUMGCommands: Starting UMG command unregistration"));
    
    FUnrealMCPCommandRegistry& Registry = FUnrealMCPCommandRegistry::Get();
    
    int32 UnregisteredCount = 0;
    for (const FString& CommandName : RegisteredCommandNames)
    {
        if (Registry.UnregisterCommand(CommandName))
        {
            UnregisteredCount++;
        }
    }
    
    RegisteredCommandNames.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("FUMGCommandRegistration::UnregisterAllUMGCommands: Unregistered %d UMG commands"), 
        UnregisteredCount);
}

void FUMGCommandRegistration::RegisterCreateWidgetBlueprintCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FCreateWidgetBlueprintCommand> Command = MakeShared<FCreateWidgetBlueprintCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterBindWidgetEventCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FBindWidgetEventCommand> Command = MakeShared<FBindWidgetEventCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterAddWidgetComponentCommand()
{
    TSharedPtr<FAddWidgetComponentCommand> Command = MakeShared<FAddWidgetComponentCommand>(FUMGService::Get());
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterSetWidgetPropertyCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FSetWidgetPropertyCommand> Command = MakeShared<FSetWidgetPropertyCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterSetTextBlockBindingCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FSetTextBlockBindingCommand> Command = MakeShared<FSetTextBlockBindingCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterCheckWidgetComponentExistsCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FCheckWidgetComponentExistsCommand> Command = MakeShared<FCheckWidgetComponentExistsCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterSetWidgetPlacementCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FSetWidgetPlacementCommand> Command = MakeShared<FSetWidgetPlacementCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterGetWidgetContainerDimensionsCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FGetWidgetContainerDimensionsCommand> Command = MakeShared<FGetWidgetContainerDimensionsCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterGetWidgetComponentLayoutCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FGetWidgetComponentLayoutCommand> Command = MakeShared<FGetWidgetComponentLayoutCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

// Widget-specific add commands - placeholders
void FUMGCommandRegistration::RegisterAddWidgetSwitcherCommand()
{
    // TODO: Implement FAddWidgetSwitcherCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddWidgetSwitcherCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddThrobberCommand()
{
    // TODO: Implement FAddThrobberCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddThrobberCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddExpandableAreaCommand()
{
    // TODO: Implement FAddExpandableAreaCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddExpandableAreaCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddMenuAnchorCommand()
{
    // TODO: Implement FAddMenuAnchorCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddMenuAnchorCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddRichTextBlockCommand()
{
    // TODO: Implement FAddRichTextBlockCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddRichTextBlockCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddSafeZoneCommand()
{
    // TODO: Implement FAddSafeZoneCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddSafeZoneCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddInvalidationBoxCommand()
{
    // TODO: Implement FAddInvalidationBoxCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddInvalidationBoxCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddInputKeySelectorCommand()
{
    // TODO: Implement FAddInputKeySelectorCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddInputKeySelectorCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddMultiLineEditableTextCommand()
{
    // TODO: Implement FAddMultiLineEditableTextCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddMultiLineEditableTextCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddSizeBoxCommand()
{
    // TODO: Implement FAddSizeBoxCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddSizeBoxCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddImageCommand()
{
    // TODO: Implement FAddImageCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddImageCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddCheckBoxCommand()
{
    // TODO: Implement FAddCheckBoxCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddCheckBoxCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddSliderCommand()
{
    // TODO: Implement FAddSliderCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddSliderCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddProgressBarCommand()
{
    // TODO: Implement FAddProgressBarCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddProgressBarCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddBorderCommand()
{
    // TODO: Implement FAddBorderCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddBorderCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddScrollBoxCommand()
{
    // TODO: Implement FAddScrollBoxCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddScrollBoxCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddSpacerCommand()
{
    // TODO: Implement FAddSpacerCommand class
    UE_LOG(LogTemp, Warning, TEXT("FUMGCommandRegistration::RegisterAddSpacerCommand: Command class not yet implemented"));
}

void FUMGCommandRegistration::RegisterAddChildWidgetCommand()
{
    // Create shared pointer to the UMG service singleton for the new architecture
    TSharedPtr<IUMGService> UMGServicePtr(&FUMGService::Get(), [](IUMGService*){});
    TSharedPtr<FAddChildWidgetCommand> Command = MakeShared<FAddChildWidgetCommand>(UMGServicePtr);
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterCreateParentChildWidgetCommand()
{
    TSharedPtr<FCreateParentChildWidgetCommand> Command = MakeShared<FCreateParentChildWidgetCommand>(FUMGService::Get());
    RegisterAndTrackCommand(Command);
}

void FUMGCommandRegistration::RegisterAndTrackCommand(TSharedPtr<IUnrealMCPCommand> Command)
{
    if (!Command.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FUMGCommandRegistration::RegisterAndTrackCommand: Invalid command"));
        return;
    }
    
    FString CommandName = Command->GetCommandName();
    if (CommandName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("FUMGCommandRegistration::RegisterAndTrackCommand: Command has empty name"));
        return;
    }
    
    FUnrealMCPCommandRegistry& Registry = FUnrealMCPCommandRegistry::Get();
    if (Registry.RegisterCommand(Command))
    {
        RegisteredCommandNames.Add(CommandName);
        UE_LOG(LogTemp, Verbose, TEXT("FUMGCommandRegistration::RegisterAndTrackCommand: Registered and tracked command '%s'"), *CommandName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FUMGCommandRegistration::RegisterAndTrackCommand: Failed to register command '%s'"), *CommandName);
    }
}
