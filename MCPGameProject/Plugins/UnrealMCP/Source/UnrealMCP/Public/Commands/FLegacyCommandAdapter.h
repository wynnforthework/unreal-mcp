#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "IUnrealMCPCommand.h"

/**
 * Legacy command adapter that provides backward compatibility for the old command API
 * Maps legacy command names to new command implementations and handles parameter transformation
 */
class UNREALMCP_API FLegacyCommandAdapter
{
public:
    /**
     * Get the singleton instance of the legacy command adapter
     * @return Reference to the singleton instance
     */
    static FLegacyCommandAdapter& Get();

    /**
     * Initialize the adapter and register legacy command mappings
     */
    void Initialize();

    /**
     * Check if a command name is a legacy command that needs adaptation
     * @param CommandName - Name of the command to check
     * @return True if this is a legacy command that needs adaptation
     */
    bool IsLegacyCommand(const FString& CommandName) const;

    /**
     * Adapt a legacy command to the new command system
     * @param LegacyCommandName - Original legacy command name
     * @param LegacyParams - Original legacy parameters
     * @return Adapted command result, or error if adaptation fails
     */
    TSharedPtr<FJsonObject> AdaptLegacyCommand(const FString& LegacyCommandName, const TSharedPtr<FJsonObject>& LegacyParams);

    /**
     * Get the new command name for a legacy command
     * @param LegacyCommandName - Legacy command name
     * @return New command name, or empty string if not found
     */
    FString GetNewCommandName(const FString& LegacyCommandName) const;

    /**
     * Transform legacy parameters to new parameter format
     * @param LegacyCommandName - Legacy command name
     * @param LegacyParams - Legacy parameters
     * @return Transformed parameters for the new command
     */
    TSharedPtr<FJsonObject> TransformParameters(const FString& LegacyCommandName, const TSharedPtr<FJsonObject>& LegacyParams) const;

    /**
     * Transform new command response to legacy response format
     * @param LegacyCommandName - Original legacy command name
     * @param NewResponse - Response from new command
     * @return Response in legacy format
     */
    TSharedPtr<FJsonObject> TransformResponse(const FString& LegacyCommandName, const TSharedPtr<FJsonObject>& NewResponse) const;

    /**
     * Log usage of legacy commands for migration tracking
     * @param LegacyCommandName - Name of the legacy command used
     * @param bShowDeprecationWarning - Whether to show deprecation warning
     */
    void LogLegacyUsage(const FString& LegacyCommandName, bool bShowDeprecationWarning = true) const;

    /**
     * Get migration guidance for a legacy command
     * @param LegacyCommandName - Legacy command name
     * @return Migration guidance text
     */
    FString GetMigrationGuidance(const FString& LegacyCommandName) const;

    /**
     * Get usage statistics for all legacy commands
     * @return Map of command names to usage counts
     */
    TMap<FString, int32> GetUsageStatistics() const;

    /**
     * Generate a migration report with usage statistics and recommendations
     * @return Formatted migration report
     */
    FString GenerateMigrationReport() const;

    /**
     * Check if deprecation warnings are enabled
     * @return True if deprecation warnings should be shown
     */
    bool ShouldShowDeprecationWarnings() const;

    /**
     * Get the current migration phase
     * @return Current migration phase (0-3)
     */
    int32 GetMigrationPhase() const;

    /**
     * Check if legacy command usage tracking is enabled
     * @return True if usage should be tracked
     */
    bool IsUsageTrackingEnabled() const;

private:
    /** Private constructor for singleton pattern */
    FLegacyCommandAdapter() = default;

    /** Whether the adapter has been initialized */
    bool bIsInitialized = false;

    /** Mapping from legacy command names to new command names */
    TMap<FString, FString> LegacyToNewCommandMap;

    /** Migration guidance for each legacy command */
    TMap<FString, FString> MigrationGuidanceMap;

    /** Commands that require parameter transformation */
    TSet<FString> CommandsRequiringTransformation;

    /** Commands that require response transformation */
    TSet<FString> CommandsRequiringResponseTransformation;

    /** Usage tracking for legacy commands */
    mutable TMap<FString, int32> LegacyUsageCount;

    /** Feature flags for controlling legacy behavior */
    mutable bool bShowDeprecationWarnings = true;
    mutable bool bTrackLegacyUsage = true;
    mutable bool bGradualMigrationMode = true;
    mutable int32 MigrationPhase = 1;
    mutable bool bLogUsageStatistics = false;

    /**
     * Register all legacy command mappings
     */
    void RegisterLegacyMappings();

    /**
     * Register a legacy command mapping
     * @param LegacyName - Legacy command name
     * @param NewName - New command name
     * @param MigrationGuidance - Migration guidance text
     * @param bRequiresParamTransform - Whether parameters need transformation
     * @param bRequiresResponseTransform - Whether response needs transformation
     */
    void RegisterLegacyMapping(const FString& LegacyName, const FString& NewName, 
                              const FString& MigrationGuidance,
                              bool bRequiresParamTransform = false, 
                              bool bRequiresResponseTransform = false);

    /**
     * Transform parameters for specific legacy commands
     */
    TSharedPtr<FJsonObject> TransformCreateBlueprintParams(const TSharedPtr<FJsonObject>& LegacyParams) const;
    TSharedPtr<FJsonObject> TransformAddComponentParams(const TSharedPtr<FJsonObject>& LegacyParams) const;
    TSharedPtr<FJsonObject> TransformSetPropertyParams(const TSharedPtr<FJsonObject>& LegacyParams) const;

    /**
     * Transform responses for specific legacy commands
     */
    TSharedPtr<FJsonObject> TransformCreateBlueprintResponse(const TSharedPtr<FJsonObject>& NewResponse) const;
    TSharedPtr<FJsonObject> TransformAddComponentResponse(const TSharedPtr<FJsonObject>& NewResponse) const;
    TSharedPtr<FJsonObject> TransformSetPropertyResponse(const TSharedPtr<FJsonObject>& NewResponse) const;

    /**
     * Create error response in legacy format
     * @param ErrorMessage - Error message
     * @return Legacy format error response
     */
    TSharedPtr<FJsonObject> CreateLegacyErrorResponse(const FString& ErrorMessage) const;

    /**
     * Load feature flags from configuration
     */
    void LoadFeatureFlags() const;

    /**
     * Check if a legacy command should be allowed based on migration phase
     * @param LegacyCommandName - Name of the legacy command
     * @return True if the command should be allowed
     */
    bool ShouldAllowLegacyCommand(const FString& LegacyCommandName) const;
};