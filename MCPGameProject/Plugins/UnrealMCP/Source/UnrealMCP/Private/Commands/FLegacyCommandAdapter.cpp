#include "Commands/FLegacyCommandAdapter.h"
#include "Commands/UnrealMCPMainDispatcher.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FLegacyCommandAdapter& FLegacyCommandAdapter::Get()
{
    static FLegacyCommandAdapter Instance;
    return Instance;
}

void FLegacyCommandAdapter::Initialize()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("FLegacyCommandAdapter::Initialize: Already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("FLegacyCommandAdapter::Initialize: Initializing legacy command adapter"));

    // Load feature flags from configuration
    LoadFeatureFlags();

    RegisterLegacyMappings();

    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("FLegacyCommandAdapter::Initialize: Legacy command adapter initialized with %d mappings (Phase: %d, Warnings: %s)"), 
           LegacyToNewCommandMap.Num(), MigrationPhase, bShowDeprecationWarnings ? TEXT("Enabled") : TEXT("Disabled"));
}

bool FLegacyCommandAdapter::IsLegacyCommand(const FString& CommandName) const
{
    return LegacyToNewCommandMap.Contains(CommandName);
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::AdaptLegacyCommand(const FString& LegacyCommandName, const TSharedPtr<FJsonObject>& LegacyParams)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("FLegacyCommandAdapter::AdaptLegacyCommand: Adapter not initialized"));
        return CreateLegacyErrorResponse(TEXT("Legacy command adapter not initialized"));
    }

    // Check if legacy command is allowed based on migration phase
    if (!ShouldAllowLegacyCommand(LegacyCommandName))
    {
        FString ErrorMessage = FString::Printf(TEXT("Legacy command '%s' is disabled in migration phase %d. %s"), 
                                             *LegacyCommandName, MigrationPhase, *GetMigrationGuidance(LegacyCommandName));
        UE_LOG(LogTemp, Error, TEXT("FLegacyCommandAdapter::AdaptLegacyCommand: %s"), *ErrorMessage);
        return CreateLegacyErrorResponse(ErrorMessage);
    }

    // Log legacy usage for tracking (with appropriate warning level based on settings)
    LogLegacyUsage(LegacyCommandName, ShouldShowDeprecationWarnings());

    // Get the new command name
    FString NewCommandName = GetNewCommandName(LegacyCommandName);
    if (NewCommandName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("FLegacyCommandAdapter::AdaptLegacyCommand: No mapping found for legacy command '%s'"), *LegacyCommandName);
        return CreateLegacyErrorResponse(FString::Printf(TEXT("Legacy command '%s' is no longer supported. %s"), 
                                                        *LegacyCommandName, *GetMigrationGuidance(LegacyCommandName)));
    }

    // Transform parameters if needed
    TSharedPtr<FJsonObject> TransformedParams = LegacyParams;
    if (CommandsRequiringTransformation.Contains(LegacyCommandName))
    {
        TransformedParams = TransformParameters(LegacyCommandName, LegacyParams);
        if (!TransformedParams.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("FLegacyCommandAdapter::AdaptLegacyCommand: Failed to transform parameters for '%s'"), *LegacyCommandName);
            return CreateLegacyErrorResponse(FString::Printf(TEXT("Failed to transform parameters for legacy command '%s'"), *LegacyCommandName));
        }
    }

    // Execute the new command
    TSharedPtr<FJsonObject> NewResponse = FUnrealMCPMainDispatcher::Get().HandleCommand(NewCommandName, TransformedParams);
    if (!NewResponse.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FLegacyCommandAdapter::AdaptLegacyCommand: New command '%s' returned invalid response"), *NewCommandName);
        return CreateLegacyErrorResponse(FString::Printf(TEXT("Internal error executing adapted command '%s'"), *NewCommandName));
    }

    // Transform response if needed
    TSharedPtr<FJsonObject> FinalResponse = NewResponse;
    if (CommandsRequiringResponseTransformation.Contains(LegacyCommandName))
    {
        FinalResponse = TransformResponse(LegacyCommandName, NewResponse);
        if (!FinalResponse.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("FLegacyCommandAdapter::AdaptLegacyCommand: Failed to transform response for '%s'"), *LegacyCommandName);
            return CreateLegacyErrorResponse(FString::Printf(TEXT("Failed to transform response for legacy command '%s'"), *LegacyCommandName));
        }
    }

    // Log usage statistics if enabled
    if (bLogUsageStatistics)
    {
        int32 UsageCount = LegacyUsageCount.FindRef(LegacyCommandName);
        if (UsageCount % 10 == 0) // Log every 10th usage
        {
            UE_LOG(LogTemp, Log, TEXT("FLegacyCommandAdapter: Legacy command '%s' has been used %d times. Consider migration."), 
                   *LegacyCommandName, UsageCount);
        }
    }

    return FinalResponse;
}

FString FLegacyCommandAdapter::GetNewCommandName(const FString& LegacyCommandName) const
{
    const FString* NewCommandName = LegacyToNewCommandMap.Find(LegacyCommandName);
    return NewCommandName ? *NewCommandName : FString();
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformParameters(const FString& LegacyCommandName, const TSharedPtr<FJsonObject>& LegacyParams) const
{
    if (!LegacyParams.IsValid())
    {
        return MakeShared<FJsonObject>();
    }

    // Handle specific parameter transformations based on legacy command
    if (LegacyCommandName == TEXT("create_blueprint"))
    {
        return TransformCreateBlueprintParams(LegacyParams);
    }
    else if (LegacyCommandName == TEXT("add_component_to_blueprint"))
    {
        return TransformAddComponentParams(LegacyParams);
    }
    else if (LegacyCommandName == TEXT("set_component_property"))
    {
        return TransformSetPropertyParams(LegacyParams);
    }

    // Default: return parameters unchanged
    return LegacyParams;
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformResponse(const FString& LegacyCommandName, const TSharedPtr<FJsonObject>& NewResponse) const
{
    if (!NewResponse.IsValid())
    {
        return CreateLegacyErrorResponse(TEXT("Invalid response from new command"));
    }

    // Handle specific response transformations based on legacy command
    if (LegacyCommandName == TEXT("create_blueprint"))
    {
        return TransformCreateBlueprintResponse(NewResponse);
    }
    else if (LegacyCommandName == TEXT("add_component_to_blueprint"))
    {
        return TransformAddComponentResponse(NewResponse);
    }
    else if (LegacyCommandName == TEXT("set_component_property"))
    {
        return TransformSetPropertyResponse(NewResponse);
    }

    // Default: return response unchanged
    return NewResponse;
}

void FLegacyCommandAdapter::LogLegacyUsage(const FString& LegacyCommandName, bool bShowDeprecationWarning) const
{
    // Track usage count only if tracking is enabled
    if (bTrackLegacyUsage)
    {
        int32& UsageCount = LegacyUsageCount.FindOrAdd(LegacyCommandName, 0);
        UsageCount++;

        // Log usage for analytics
        UE_LOG(LogTemp, Log, TEXT("FLegacyCommandAdapter: Legacy command '%s' used (count: %d)"), *LegacyCommandName, UsageCount);
    }

    // Show deprecation warning if requested and not in phase 0
    if (bShowDeprecationWarning && MigrationPhase > 0)
    {
        FString MigrationGuidance = GetMigrationGuidance(LegacyCommandName);
        
        // Use different log levels based on migration phase
        switch (MigrationPhase)
        {
            case 1:
                UE_LOG(LogTemp, Warning, TEXT("DEPRECATED: Legacy command '%s' is deprecated. %s"), *LegacyCommandName, *MigrationGuidance);
                break;
            case 2:
                UE_LOG(LogTemp, Error, TEXT("LEGACY WARNING: Command '%s' will be disabled in future versions. %s"), *LegacyCommandName, *MigrationGuidance);
                break;
            case 3:
                UE_LOG(LogTemp, Fatal, TEXT("LEGACY ERROR: Command '%s' should be disabled. %s"), *LegacyCommandName, *MigrationGuidance);
                break;
            default:
                UE_LOG(LogTemp, Warning, TEXT("DEPRECATED: Legacy command '%s' is deprecated. %s"), *LegacyCommandName, *MigrationGuidance);
                break;
        }
    }
}

FString FLegacyCommandAdapter::GetMigrationGuidance(const FString& LegacyCommandName) const
{
    const FString* Guidance = MigrationGuidanceMap.Find(LegacyCommandName);
    if (Guidance && !Guidance->IsEmpty())
    {
        return *Guidance;
    }

    FString NewCommandName = GetNewCommandName(LegacyCommandName);
    if (!NewCommandName.IsEmpty())
    {
        return FString::Printf(TEXT("Use '%s' instead."), *NewCommandName);
    }

    return TEXT("Please refer to the migration documentation for alternatives.");
}

void FLegacyCommandAdapter::RegisterLegacyMappings()
{
    // Blueprint commands
    RegisterLegacyMapping(
        TEXT("create_blueprint"),
        TEXT("CreateBlueprintCommand"),
        TEXT("Use 'CreateBlueprintCommand' with the new parameter structure. The 'name' parameter now supports full paths."),
        true,  // Requires parameter transformation
        true   // Requires response transformation
    );

    RegisterLegacyMapping(
        TEXT("add_component_to_blueprint"),
        TEXT("AddComponentToBlueprintCommand"),
        TEXT("Use 'AddComponentToBlueprintCommand' with the new component factory system."),
        true,  // Requires parameter transformation
        false  // No response transformation needed
    );

    RegisterLegacyMapping(
        TEXT("set_component_property"),
        TEXT("SetComponentPropertyCommand"),
        TEXT("Use 'SetComponentPropertyCommand' with the new type-safe property system."),
        true,  // Requires parameter transformation
        true   // Requires response transformation
    );

    RegisterLegacyMapping(
        TEXT("compile_blueprint"),
        TEXT("CompileBlueprintCommand"),
        TEXT("Use 'CompileBlueprintCommand' directly - no parameter changes required."),
        false, // No parameter transformation needed
        false  // No response transformation needed
    );

    RegisterLegacyMapping(
        TEXT("set_physics_properties"),
        TEXT("SetPhysicsPropertiesCommand"),
        TEXT("Use 'SetPhysicsPropertiesCommand' with the new physics service."),
        false, // No parameter transformation needed
        false  // No response transformation needed
    );

    // Add more legacy mappings as needed...
    
    UE_LOG(LogTemp, Log, TEXT("FLegacyCommandAdapter::RegisterLegacyMappings: Registered %d legacy command mappings"), 
           LegacyToNewCommandMap.Num());
}

void FLegacyCommandAdapter::RegisterLegacyMapping(const FString& LegacyName, const FString& NewName, 
                                                 const FString& MigrationGuidance,
                                                 bool bRequiresParamTransform, 
                                                 bool bRequiresResponseTransform)
{
    LegacyToNewCommandMap.Add(LegacyName, NewName);
    MigrationGuidanceMap.Add(LegacyName, MigrationGuidance);

    if (bRequiresParamTransform)
    {
        CommandsRequiringTransformation.Add(LegacyName);
    }

    if (bRequiresResponseTransform)
    {
        CommandsRequiringResponseTransformation.Add(LegacyName);
    }

    UE_LOG(LogTemp, Verbose, TEXT("FLegacyCommandAdapter::RegisterLegacyMapping: '%s' -> '%s'"), *LegacyName, *NewName);
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformCreateBlueprintParams(const TSharedPtr<FJsonObject>& LegacyParams) const
{
    TSharedPtr<FJsonObject> TransformedParams = MakeShared<FJsonObject>();

    // Copy all existing parameters
    for (const auto& Pair : LegacyParams->Values)
    {
        TransformedParams->SetField(Pair.Key, Pair.Value);
    }

    // Handle any specific parameter transformations for create_blueprint
    // For example, if the old API used different parameter names
    FString OldName;
    if (LegacyParams->TryGetStringField(TEXT("blueprint_name"), OldName))
    {
        TransformedParams->SetStringField(TEXT("name"), OldName);
        TransformedParams->RemoveField(TEXT("blueprint_name"));
    }

    return TransformedParams;
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformAddComponentParams(const TSharedPtr<FJsonObject>& LegacyParams) const
{
    TSharedPtr<FJsonObject> TransformedParams = MakeShared<FJsonObject>();

    // Copy all existing parameters
    for (const auto& Pair : LegacyParams->Values)
    {
        TransformedParams->SetField(Pair.Key, Pair.Value);
    }

    // Handle any specific parameter transformations for add_component_to_blueprint
    // The new system might expect different parameter names or formats

    return TransformedParams;
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformSetPropertyParams(const TSharedPtr<FJsonObject>& LegacyParams) const
{
    TSharedPtr<FJsonObject> TransformedParams = MakeShared<FJsonObject>();

    // Copy all existing parameters
    for (const auto& Pair : LegacyParams->Values)
    {
        TransformedParams->SetField(Pair.Key, Pair.Value);
    }

    // Handle any specific parameter transformations for set_component_property
    // The new system might use a different property setting mechanism

    return TransformedParams;
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformCreateBlueprintResponse(const TSharedPtr<FJsonObject>& NewResponse) const
{
    TSharedPtr<FJsonObject> LegacyResponse = MakeShared<FJsonObject>();

    // Transform new response format to legacy format
    bool bSuccess = false;
    NewResponse->TryGetBoolField(TEXT("success"), bSuccess);
    LegacyResponse->SetBoolField(TEXT("success"), bSuccess);

    if (bSuccess)
    {
        // Copy relevant fields to legacy format
        FString BlueprintPath;
        if (NewResponse->TryGetStringField(TEXT("blueprint_path"), BlueprintPath))
        {
            LegacyResponse->SetStringField(TEXT("path"), BlueprintPath);
        }

        FString BlueprintName;
        if (NewResponse->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
        {
            LegacyResponse->SetStringField(TEXT("name"), BlueprintName);
        }
    }
    else
    {
        // Copy error information
        FString ErrorMessage;
        if (NewResponse->TryGetStringField(TEXT("error"), ErrorMessage))
        {
            LegacyResponse->SetStringField(TEXT("error"), ErrorMessage);
        }
    }

    return LegacyResponse;
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformAddComponentResponse(const TSharedPtr<FJsonObject>& NewResponse) const
{
    TSharedPtr<FJsonObject> LegacyResponse = MakeShared<FJsonObject>();

    // Transform new response format to legacy format for add component
    bool bSuccess = false;
    NewResponse->TryGetBoolField(TEXT("success"), bSuccess);
    LegacyResponse->SetBoolField(TEXT("success"), bSuccess);

    if (bSuccess)
    {
        // Copy component information to legacy format
        FString ComponentName;
        if (NewResponse->TryGetStringField(TEXT("component_name"), ComponentName))
        {
            LegacyResponse->SetStringField(TEXT("component_name"), ComponentName);
        }

        FString ComponentType;
        if (NewResponse->TryGetStringField(TEXT("component_type"), ComponentType))
        {
            LegacyResponse->SetStringField(TEXT("component_type"), ComponentType);
        }
    }
    else
    {
        // Copy error information
        FString ErrorMessage;
        if (NewResponse->TryGetStringField(TEXT("error"), ErrorMessage))
        {
            LegacyResponse->SetStringField(TEXT("error"), ErrorMessage);
        }
    }

    return LegacyResponse;
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::TransformSetPropertyResponse(const TSharedPtr<FJsonObject>& NewResponse) const
{
    TSharedPtr<FJsonObject> LegacyResponse = MakeShared<FJsonObject>();

    // Transform new response format to legacy format for set property
    bool bSuccess = false;
    NewResponse->TryGetBoolField(TEXT("success"), bSuccess);
    LegacyResponse->SetBoolField(TEXT("success"), bSuccess);

    if (bSuccess)
    {
        // Copy property setting results to legacy format
        const TArray<TSharedPtr<FJsonValue>>* SuccessArray = nullptr;
        if (NewResponse->TryGetArrayField(TEXT("success_properties"), SuccessArray))
        {
            LegacyResponse->SetArrayField(TEXT("success_properties"), *SuccessArray);
        }

        const TArray<TSharedPtr<FJsonValue>>* FailedArray = nullptr;
        if (NewResponse->TryGetArrayField(TEXT("failed_properties"), FailedArray))
        {
            LegacyResponse->SetArrayField(TEXT("failed_properties"), *FailedArray);
        }
    }
    else
    {
        // Copy error information
        FString ErrorMessage;
        if (NewResponse->TryGetStringField(TEXT("error"), ErrorMessage))
        {
            LegacyResponse->SetStringField(TEXT("error"), ErrorMessage);
        }
    }

    return LegacyResponse;
}

TSharedPtr<FJsonObject> FLegacyCommandAdapter::CreateLegacyErrorResponse(const FString& ErrorMessage) const
{
    TSharedPtr<FJsonObject> ErrorResponse = MakeShared<FJsonObject>();
    ErrorResponse->SetBoolField(TEXT("success"), false);
    ErrorResponse->SetStringField(TEXT("error"), ErrorMessage);
    return ErrorResponse;
}

TMap<FString, int32> FLegacyCommandAdapter::GetUsageStatistics() const
{
    return LegacyUsageCount;
}

FString FLegacyCommandAdapter::GenerateMigrationReport() const
{
    FString Report;
    Report += TEXT("=== Legacy Command Migration Report ===\n\n");
    
    if (LegacyUsageCount.Num() == 0)
    {
        Report += TEXT("No legacy commands have been used.\n");
        Report += TEXT("Your codebase appears to be fully migrated!\n\n");
    }
    else
    {
        Report += FString::Printf(TEXT("Total legacy commands used: %d\n"), LegacyUsageCount.Num());
        Report += FString::Printf(TEXT("Migration Phase: %d\n"), MigrationPhase);
        Report += FString::Printf(TEXT("Deprecation Warnings: %s\n\n"), bShowDeprecationWarnings ? TEXT("Enabled") : TEXT("Disabled"));
        
        Report += TEXT("Command Usage Statistics:\n");
        Report += TEXT("------------------------\n");
        
        // Sort commands by usage count (descending)
        TArray<TPair<FString, int32>> SortedUsage;
        for (const auto& Pair : LegacyUsageCount)
        {
            SortedUsage.Add(TPair<FString, int32>(Pair.Key, Pair.Value));
        }
        SortedUsage.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B) {
            return A.Value > B.Value;
        });
        
        for (const auto& Pair : SortedUsage)
        {
            FString NewCommandName = GetNewCommandName(Pair.Key);
            Report += FString::Printf(TEXT("• %s: %d uses -> Migrate to: %s\n"), 
                                    *Pair.Key, Pair.Value, *NewCommandName);
        }
        
        Report += TEXT("\nMigration Recommendations:\n");
        Report += TEXT("-------------------------\n");
        
        for (const auto& Pair : SortedUsage)
        {
            FString Guidance = GetMigrationGuidance(Pair.Key);
            Report += FString::Printf(TEXT("• %s: %s\n"), *Pair.Key, *Guidance);
        }
        
        Report += TEXT("\nNext Steps:\n");
        Report += TEXT("-----------\n");
        Report += TEXT("1. Review the migration documentation at Documentation/LegacyCommandMigration.md\n");
        Report += TEXT("2. Update your code to use the new command names\n");
        Report += TEXT("3. Test thoroughly after migration\n");
        Report += TEXT("4. Consider enabling stricter migration phases\n\n");
    }
    
    Report += TEXT("For detailed migration guidance, see: Documentation/LegacyCommandMigration.md\n");
    
    return Report;
}

bool FLegacyCommandAdapter::ShouldShowDeprecationWarnings() const
{
    return bShowDeprecationWarnings;
}

int32 FLegacyCommandAdapter::GetMigrationPhase() const
{
    return MigrationPhase;
}

bool FLegacyCommandAdapter::IsUsageTrackingEnabled() const
{
    return bTrackLegacyUsage;
}

void FLegacyCommandAdapter::LoadFeatureFlags() const
{
    // Load configuration from DefaultEngine.ini
    // Note: In a real implementation, you would use GConfig to read from ini files
    // For now, we'll use default values that can be overridden
    
    // Default values (can be overridden by configuration)
    bShowDeprecationWarnings = true;
    bTrackLegacyUsage = true;
    bGradualMigrationMode = true;
    MigrationPhase = 1;
    bLogUsageStatistics = false;
    
    // In a full implementation, you would read from configuration like this:
    // GConfig->GetBool(TEXT("/Script/UnrealMCP.LegacyCommandAdapter"), TEXT("bShowDeprecationWarnings"), bShowDeprecationWarnings, GEngineIni);
    // GConfig->GetBool(TEXT("/Script/UnrealMCP.LegacyCommandAdapter"), TEXT("bTrackLegacyUsage"), bTrackLegacyUsage, GEngineIni);
    // GConfig->GetBool(TEXT("/Script/UnrealMCP.LegacyCommandAdapter"), TEXT("bGradualMigrationMode"), bGradualMigrationMode, GEngineIni);
    // GConfig->GetInt(TEXT("/Script/UnrealMCP.LegacyCommandAdapter"), TEXT("MigrationPhase"), MigrationPhase, GEngineIni);
    // GConfig->GetBool(TEXT("/Script/UnrealMCP.LegacyCommandAdapter"), TEXT("bLogUsageStatistics"), bLogUsageStatistics, GEngineIni);
    
    UE_LOG(LogTemp, Log, TEXT("FLegacyCommandAdapter::LoadFeatureFlags: Loaded feature flags - Phase: %d, Warnings: %s, Tracking: %s"), 
           MigrationPhase, bShowDeprecationWarnings ? TEXT("On") : TEXT("Off"), bTrackLegacyUsage ? TEXT("On") : TEXT("Off"));
}

bool FLegacyCommandAdapter::ShouldAllowLegacyCommand(const FString& LegacyCommandName) const
{
    // Check migration phase to determine if legacy command should be allowed
    switch (MigrationPhase)
    {
        case 0: // Phase 0: All legacy commands allowed without warnings
            return true;
            
        case 1: // Phase 1: All legacy commands allowed with warnings (default)
            return true;
            
        case 2: // Phase 2: Legacy commands require explicit enablement
            // In this phase, you might check for specific enablement flags
            // For now, we'll allow all commands but log more aggressively
            UE_LOG(LogTemp, Warning, TEXT("FLegacyCommandAdapter: Legacy command '%s' used in Phase 2 - consider migration"), *LegacyCommandName);
            return true;
            
        case 3: // Phase 3: Legacy commands disabled
            UE_LOG(LogTemp, Error, TEXT("FLegacyCommandAdapter: Legacy command '%s' disabled in Phase 3"), *LegacyCommandName);
            return false;
            
        default:
            UE_LOG(LogTemp, Warning, TEXT("FLegacyCommandAdapter: Unknown migration phase %d, defaulting to Phase 1"), MigrationPhase);
            return true;
    }
}