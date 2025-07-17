#include "Services/BlueprintService.h"
#include "Services/ComponentService.h"
#include "Services/PropertyService.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"

// Blueprint Cache Implementation
UBlueprint* FBlueprintCache::GetBlueprint(const FString& BlueprintName)
{
    FScopeLock Lock(&CacheLock);
    
    // Update statistics
    UpdateStats(false); // Assume miss initially
    
    if (TWeakObjectPtr<UBlueprint>* CachedPtr = CachedBlueprints.Find(BlueprintName))
    {
        if (CachedPtr->IsValid())
        {
            // Update to hit since we found a valid entry
            CacheStats.CacheHits++;
            CacheStats.CacheMisses--;
            
            UE_LOG(LogTemp, Verbose, TEXT("FBlueprintCache: Cache hit for blueprint '%s'"), *BlueprintName);
            return CachedPtr->Get();
        }
        else
        {
            // Remove invalid entry
            CachedBlueprints.Remove(BlueprintName);
            CacheStats.CachedCount = CachedBlueprints.Num();
            UE_LOG(LogTemp, Verbose, TEXT("FBlueprintCache: Removed invalid cache entry for blueprint '%s'"), *BlueprintName);
        }
    }
    
    return nullptr;
}

void FBlueprintCache::CacheBlueprint(const FString& BlueprintName, UBlueprint* Blueprint)
{
    if (!Blueprint)
    {
        return;
    }
    
    FScopeLock Lock(&CacheLock);
    CachedBlueprints.Add(BlueprintName, Blueprint);
    CacheStats.CachedCount = CachedBlueprints.Num();
    UE_LOG(LogTemp, Verbose, TEXT("FBlueprintCache: Cached blueprint '%s'"), *BlueprintName);
}

void FBlueprintCache::InvalidateBlueprint(const FString& BlueprintName)
{
    FScopeLock Lock(&CacheLock);
    if (CachedBlueprints.Remove(BlueprintName) > 0)
    {
        CacheStats.InvalidatedCount++;
        CacheStats.CachedCount = CachedBlueprints.Num();
        UE_LOG(LogTemp, Verbose, TEXT("FBlueprintCache: Invalidated cache for blueprint '%s'"), *BlueprintName);
    }
}

void FBlueprintCache::ClearCache()
{
    FScopeLock Lock(&CacheLock);
    int32 ClearedCount = CachedBlueprints.Num();
    CachedBlueprints.Empty();
    CacheStats.CachedCount = 0;
    UE_LOG(LogTemp, Log, TEXT("FBlueprintCache: Cleared %d cached blueprints"), ClearedCount);
}

void FBlueprintCache::WarmCache(const TArray<FString>& BlueprintNames)
{
    UE_LOG(LogTemp, Log, TEXT("FBlueprintCache: Warming cache with %d blueprints"), BlueprintNames.Num());
    
    for (const FString& BlueprintName : BlueprintNames)
    {
        // Check if already cached
        if (IsCached(BlueprintName))
        {
            continue;
        }
        
        // Try to find and cache the blueprint
        UBlueprint* FoundBlueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
        if (FoundBlueprint)
        {
            CacheBlueprint(BlueprintName, FoundBlueprint);
            UE_LOG(LogTemp, Verbose, TEXT("FBlueprintCache: Warmed cache with blueprint '%s'"), *BlueprintName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FBlueprintCache: Could not find blueprint '%s' for cache warming"), *BlueprintName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintCache: Cache warming complete. %d blueprints cached"), GetCachedCount());
}

void FBlueprintCache::WarmCacheWithCommonBlueprints()
{
    UE_LOG(LogTemp, Log, TEXT("FBlueprintCache: Warming cache with common blueprints"));
    
    // Common blueprint names that are frequently used
    TArray<FString> CommonBlueprints = {
        TEXT("BP_PlayerController"),
        TEXT("BP_GameMode"),
        TEXT("BP_GameState"),
        TEXT("BP_PlayerState"),
        TEXT("BP_Character"),
        TEXT("BP_Pawn"),
        TEXT("BP_Actor"),
        TEXT("BP_HUD"),
        TEXT("BP_Widget"),
        TEXT("BP_UserWidget"),
        TEXT("ThirdPersonCharacter"),
        TEXT("BP_ThirdPersonCharacter"),
        TEXT("FirstPersonCharacter"),
        TEXT("BP_FirstPersonCharacter")
    };
    
    WarmCache(CommonBlueprints);
}

FBlueprintCacheStats FBlueprintCache::GetCacheStats() const
{
    FScopeLock Lock(&CacheLock);
    FBlueprintCacheStats StatsCopy = CacheStats;
    StatsCopy.CachedCount = CachedBlueprints.Num();
    return StatsCopy;
}

void FBlueprintCache::ResetCacheStats()
{
    FScopeLock Lock(&CacheLock);
    CacheStats.Reset();
    CacheStats.CachedCount = CachedBlueprints.Num();
    UE_LOG(LogTemp, Log, TEXT("FBlueprintCache: Cache statistics reset"));
}

int32 FBlueprintCache::GetCachedCount() const
{
    FScopeLock Lock(&CacheLock);
    return CachedBlueprints.Num();
}

bool FBlueprintCache::IsCached(const FString& BlueprintName) const
{
    FScopeLock Lock(&CacheLock);
    
    if (const TWeakObjectPtr<UBlueprint>* CachedPtr = CachedBlueprints.Find(BlueprintName))
    {
        return CachedPtr->IsValid();
    }
    
    return false;
}

void FBlueprintCache::UpdateStats(bool bWasHit) const
{
    // This method assumes the lock is already held
    CacheStats.TotalRequests++;
    if (bWasHit)
    {
        CacheStats.CacheHits++;
    }
    else
    {
        CacheStats.CacheMisses++;
    }
}

int32 FBlueprintCache::CleanupInvalidEntries()
{
    // This method assumes the lock is already held
    int32 CleanedCount = 0;
    
    for (auto It = CachedBlueprints.CreateIterator(); It; ++It)
    {
        if (!It.Value().IsValid())
        {
            It.RemoveCurrent();
            CleanedCount++;
        }
    }
    
    if (CleanedCount > 0)
    {
        CacheStats.CachedCount = CachedBlueprints.Num();
        UE_LOG(LogTemp, Log, TEXT("FBlueprintCache: Cleaned up %d invalid cache entries"), CleanedCount);
    }
    
    return CleanedCount;
}

// Blueprint Service Implementation
FBlueprintService& FBlueprintService::Get()
{
    static FBlueprintService Instance;
    return Instance;
}

UBlueprint* FBlueprintService::CreateBlueprint(const FBlueprintCreationParams& Params)
{
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::CreateBlueprint: Creating blueprint '%s'"), *Params.Name);
    
    // Validate parameters
    FString ValidationError;
    if (!Params.IsValid(ValidationError))
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::CreateBlueprint: Invalid parameters - %s"), *ValidationError);
        return nullptr;
    }
    
    // Normalize the blueprint path
    FString NormalizedPath;
    FString PathError;
    if (!NormalizeBlueprintPath(Params.FolderPath, NormalizedPath, PathError))
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::CreateBlueprint: Path normalization failed - %s"), *PathError);
        return nullptr;
    }
    
    // Build full asset path
    FString FullAssetPath = NormalizedPath + Params.Name;
    
    // Check if blueprint already exists
    if (UEditorAssetLibrary::DoesAssetExist(FullAssetPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("FBlueprintService::CreateBlueprint: Blueprint already exists at '%s'"), *FullAssetPath);
        UBlueprint* ExistingBlueprint = Cast<UBlueprint>(UEditorAssetLibrary::LoadAsset(FullAssetPath));
        if (ExistingBlueprint)
        {
            BlueprintCache.CacheBlueprint(Params.Name, ExistingBlueprint);
        }
        return ExistingBlueprint;
    }
    
    // Create directory structure if needed
    FString DirectoryError;
    if (!CreateDirectoryStructure(NormalizedPath, DirectoryError))
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::CreateBlueprint: Failed to create directory structure - %s"), *DirectoryError);
        return nullptr;
    }
    
    // Resolve parent class
    UClass* ParentClass = Params.ParentClass;
    if (!ParentClass)
    {
        ParentClass = AActor::StaticClass(); // Default to Actor
        UE_LOG(LogTemp, Log, TEXT("FBlueprintService::CreateBlueprint: Using default parent class AActor"));
    }
    
    // Create the package
    UObject* Package = CreatePackage(*FullAssetPath);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::CreateBlueprint: Failed to create package for path '%s'"), *FullAssetPath);
        return nullptr;
    }
    
    // Create the blueprint using FKismetEditorUtilities
    UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
        ParentClass,
        Package,
        *Params.Name,
        BPTYPE_Normal,
        UBlueprint::StaticClass(),
        UBlueprintGeneratedClass::StaticClass(),
        NAME_None
    );
    
    if (!NewBlueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::CreateBlueprint: Failed to create blueprint"));
        return nullptr;
    }
    
    // Notify the asset registry
    FAssetRegistryModule::AssetCreated(NewBlueprint);
    
    // Mark the package dirty
    Package->MarkPackageDirty();
    
    // Compile if requested
    if (Params.bCompileOnCreation)
    {
        FString CompileError;
        if (!CompileBlueprint(NewBlueprint, CompileError))
        {
            UE_LOG(LogTemp, Warning, TEXT("FBlueprintService::CreateBlueprint: Blueprint compilation failed - %s"), *CompileError);
        }
    }
    
    // Save the asset
    if (UEditorAssetLibrary::SaveLoadedAsset(NewBlueprint))
    {
        UE_LOG(LogTemp, Log, TEXT("FBlueprintService::CreateBlueprint: Successfully saved blueprint '%s'"), *FullAssetPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FBlueprintService::CreateBlueprint: Failed to save blueprint '%s'"), *FullAssetPath);
    }
    
    // Cache the blueprint
    BlueprintCache.CacheBlueprint(Params.Name, NewBlueprint);
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::CreateBlueprint: Successfully created blueprint '%s'"), *Params.Name);
    return NewBlueprint;
}

bool FBlueprintService::AddComponentToBlueprint(UBlueprint* Blueprint, const FComponentCreationParams& Params)
{
    // Delegate to ComponentService for component operations
    bool bResult = FComponentService::Get().AddComponentToBlueprint(Blueprint, Params);
    
    if (bResult)
    {
        // Invalidate cache since blueprint was modified
        BlueprintCache.InvalidateBlueprint(Blueprint->GetName());
    }
    
    return bResult;
}

bool FBlueprintService::CompileBlueprint(UBlueprint* Blueprint, FString& OutError)
{
    if (!Blueprint)
    {
        OutError = TEXT("Invalid blueprint");
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::CompileBlueprint: Invalid blueprint"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::CompileBlueprint: Compiling blueprint '%s'"), *Blueprint->GetName());
    
    // Clear any existing compilation errors
    Blueprint->Status = BS_Unknown;
    
    // Compile the blueprint
    FKismetEditorUtilities::CompileBlueprint(Blueprint);
    
    // Check compilation result
    if (Blueprint->Status == BS_Error)
    {
        OutError = TEXT("Blueprint compilation failed with errors");
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::CompileBlueprint: Compilation failed for blueprint '%s'"), *Blueprint->GetName());
        
        // CompileLog is not available in UE 5.6, using basic error reporting
        
        return false;
    }
    else if (Blueprint->Status == BS_UpToDateWithWarnings)
    {
        UE_LOG(LogTemp, Warning, TEXT("FBlueprintService::CompileBlueprint: Blueprint '%s' compiled with warnings"), *Blueprint->GetName());
        
        // CompileLog is not available in UE 5.6, using basic warning reporting
    }
    
    // Invalidate cache since blueprint was modified
    BlueprintCache.InvalidateBlueprint(Blueprint->GetName());
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::CompileBlueprint: Successfully compiled blueprint '%s'"), *Blueprint->GetName());
    return true;
}

UBlueprint* FBlueprintService::FindBlueprint(const FString& BlueprintName)
{
    UE_LOG(LogTemp, Verbose, TEXT("FBlueprintService::FindBlueprint: Looking for blueprint '%s'"), *BlueprintName);
    
    // Check cache first
    UBlueprint* CachedBlueprint = BlueprintCache.GetBlueprint(BlueprintName);
    if (CachedBlueprint)
    {
        UE_LOG(LogTemp, Verbose, TEXT("FBlueprintService::FindBlueprint: Found blueprint '%s' in cache"), *BlueprintName);
        return CachedBlueprint;
    }
    
    // Use common utils to find blueprint
    UBlueprint* FoundBlueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (FoundBlueprint)
    {
        // Cache for future lookups
        BlueprintCache.CacheBlueprint(BlueprintName, FoundBlueprint);
        UE_LOG(LogTemp, Verbose, TEXT("FBlueprintService::FindBlueprint: Found and cached blueprint '%s'"), *BlueprintName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FBlueprintService::FindBlueprint: Blueprint '%s' not found"), *BlueprintName);
    }
    
    return FoundBlueprint;
}

bool FBlueprintService::AddVariableToBlueprint(UBlueprint* Blueprint, const FString& VariableName, const FString& VariableType, bool bIsExposed)
{
    if (!Blueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::AddVariableToBlueprint: Invalid blueprint"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::AddVariableToBlueprint: Adding variable '%s' of type '%s' to blueprint '%s'"), 
        *VariableName, *VariableType, *Blueprint->GetName());
    
    // Resolve variable type
    UObject* TypeObject = ResolveVariableType(VariableType);
    if (!TypeObject)
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::AddVariableToBlueprint: Unknown variable type '%s'"), *VariableType);
        return false;
    }
    
    // Create variable description
    FBPVariableDescription NewVar;
    NewVar.VarName = *VariableName;
    NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Object; // Default, will be adjusted based on type
    
    // Set type based on resolved type object
    if (UClass* ClassType = Cast<UClass>(TypeObject))
    {
        NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Object;
        NewVar.VarType.PinSubCategoryObject = ClassType;
    }
    else if (UScriptStruct* StructType = Cast<UScriptStruct>(TypeObject))
    {
        NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Struct;
        NewVar.VarType.PinSubCategoryObject = StructType;
    }
    else
    {
        // Handle basic types
        if (VariableType == TEXT("Boolean") || VariableType == TEXT("bool"))
        {
            NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
        }
        else if (VariableType == TEXT("Integer") || VariableType == TEXT("int") || VariableType == TEXT("int32"))
        {
            NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Int;
        }
        else if (VariableType == TEXT("Float") || VariableType == TEXT("float"))
        {
            NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Real;
            NewVar.VarType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
        }
        else if (VariableType == TEXT("String") || VariableType == TEXT("FString"))
        {
            NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_String;
        }
        else if (VariableType == TEXT("Vector") || VariableType == TEXT("FVector"))
        {
            NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            NewVar.VarType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
        }
        else if (VariableType == TEXT("Rotator") || VariableType == TEXT("FRotator"))
        {
            NewVar.VarType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            NewVar.VarType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FBlueprintService::AddVariableToBlueprint: Unsupported basic type '%s'"), *VariableType);
            return false;
        }
    }
    
    // Set exposure
    if (bIsExposed)
    {
        NewVar.PropertyFlags |= CPF_BlueprintVisible;
        NewVar.PropertyFlags |= CPF_Edit;
    }
    
    // Add variable to blueprint
    FBlueprintEditorUtils::AddMemberVariable(Blueprint, NewVar.VarName, NewVar.VarType);
    
    // Mark blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    
    // Invalidate cache since blueprint was modified
    BlueprintCache.InvalidateBlueprint(Blueprint->GetName());
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::AddVariableToBlueprint: Successfully added variable '%s'"), *VariableName);
    return true;
}

bool FBlueprintService::SetBlueprintProperty(UBlueprint* Blueprint, const FString& PropertyName, const TSharedPtr<FJsonValue>& PropertyValue)
{
    if (!Blueprint)
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::SetBlueprintProperty: Invalid blueprint"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::SetBlueprintProperty: Setting property '%s' on blueprint '%s'"), 
        *PropertyName, *Blueprint->GetName());
    
    // Get the blueprint's default object
    UObject* DefaultObject = Blueprint->GeneratedClass ? Blueprint->GeneratedClass->GetDefaultObject() : nullptr;
    if (!DefaultObject)
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::SetBlueprintProperty: No default object available"));
        return false;
    }
    
    // Set the property using PropertyService
    FString ErrorMessage;
    if (!FPropertyService::Get().SetObjectProperty(DefaultObject, PropertyName, PropertyValue, ErrorMessage))
    {
        UE_LOG(LogTemp, Error, TEXT("FBlueprintService::SetBlueprintProperty: Failed to set property - %s"), *ErrorMessage);
        return false;
    }
    
    // Mark blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    
    // Invalidate cache since blueprint was modified
    BlueprintCache.InvalidateBlueprint(Blueprint->GetName());
    
    UE_LOG(LogTemp, Log, TEXT("FBlueprintService::SetBlueprintProperty: Successfully set property '%s'"), *PropertyName);
    return true;
}

// Private helper methods
UClass* FBlueprintService::ResolveParentClass(const FString& ParentClassName) const
{
    if (ParentClassName.IsEmpty())
    {
        return AActor::StaticClass(); // Default
    }
    
    FString ClassName = ParentClassName;
    
    // Add appropriate prefix if not present
    if (!ClassName.StartsWith(TEXT("A")) && !ClassName.StartsWith(TEXT("U")))
    {
        if (ClassName.EndsWith(TEXT("Component")))
        {
            ClassName = TEXT("U") + ClassName;
        }
        else
        {
            ClassName = TEXT("A") + ClassName;
        }
    }
    
    // Try direct StaticClass lookup for common classes
    if (ClassName == TEXT("APawn"))
    {
        return APawn::StaticClass();
    }
    else if (ClassName == TEXT("AActor"))
    {
        return AActor::StaticClass();
    }
    else if (ClassName == TEXT("ACharacter"))
    {
        return ACharacter::StaticClass();
    }
    else if (ClassName == TEXT("APlayerController"))
    {
        return APlayerController::StaticClass();
    }
    else if (ClassName == TEXT("AGameModeBase"))
    {
        return AGameModeBase::StaticClass();
    }
    else if (ClassName == TEXT("UActorComponent"))
    {
        return UActorComponent::StaticClass();
    }
    else if (ClassName == TEXT("USceneComponent"))
    {
        return USceneComponent::StaticClass();
    }
    
    // Try loading from common module paths
    TArray<FString> ModulePaths = {
        TEXT("/Script/Engine"),
        TEXT("/Script/GameplayAbilities"),
        TEXT("/Script/AIModule"),
        TEXT("/Script/Game"),
        TEXT("/Script/CoreUObject")
    };
    
    for (const FString& ModulePath : ModulePaths)
    {
        const FString ClassPath = FString::Printf(TEXT("%s.%s"), *ModulePath, *ClassName);
        if (UClass* FoundClass = LoadClass<UObject>(nullptr, *ClassPath))
        {
            return FoundClass;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("FBlueprintService::ResolveParentClass: Could not resolve parent class '%s'"), *ParentClassName);
    return AActor::StaticClass(); // Fallback to Actor
}

bool FBlueprintService::CreateDirectoryStructure(const FString& FolderPath, FString& OutError) const
{
    if (FolderPath.IsEmpty() || UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
    {
        return true; // Already exists or not needed
    }
    
    // Parse folder levels
    FString CleanPath = FolderPath;
    if (CleanPath.StartsWith(TEXT("/Game/")))
    {
        CleanPath = CleanPath.RightChop(6); // Remove "/Game/"
    }
    
    if (CleanPath.IsEmpty())
    {
        return true;
    }
    
    TArray<FString> FolderLevels;
    CleanPath.ParseIntoArray(FolderLevels, TEXT("/"));
    
    FString CurrentPath = TEXT("/Game/");
    for (const FString& FolderLevel : FolderLevels)
    {
        CurrentPath += FolderLevel + TEXT("/");
        if (!UEditorAssetLibrary::DoesDirectoryExist(CurrentPath))
        {
            if (!UEditorAssetLibrary::MakeDirectory(CurrentPath))
            {
                OutError = FString::Printf(TEXT("Failed to create directory: %s"), *CurrentPath);
                return false;
            }
            UE_LOG(LogTemp, Log, TEXT("FBlueprintService::CreateDirectoryStructure: Created directory '%s'"), *CurrentPath);
        }
    }
    
    return true;
}

bool FBlueprintService::NormalizeBlueprintPath(const FString& InputPath, FString& OutNormalizedPath, FString& OutError) const
{
    FString CleanPath = InputPath;
    
    // Remove leading slash
    if (CleanPath.StartsWith(TEXT("/")))
    {
        CleanPath = CleanPath.RightChop(1);
    }
    
    // Remove Content/ prefix
    if (CleanPath.StartsWith(TEXT("Content/")))
    {
        CleanPath = CleanPath.RightChop(8);
    }
    
    // Remove Game/ prefix
    if (CleanPath.StartsWith(TEXT("Game/")))
    {
        CleanPath = CleanPath.RightChop(5);
    }
    
    // Remove trailing slash
    if (CleanPath.EndsWith(TEXT("/")))
    {
        CleanPath = CleanPath.LeftChop(1);
    }
    
    // Build normalized path
    OutNormalizedPath = TEXT("/Game/");
    if (!CleanPath.IsEmpty())
    {
        OutNormalizedPath += CleanPath + TEXT("/");
    }
    
    return true;
}

UObject* FBlueprintService::ResolveVariableType(const FString& TypeString) const
{
    // Handle basic types (these don't need UObject resolution)
    if (TypeString == TEXT("Boolean") || TypeString == TEXT("bool") ||
        TypeString == TEXT("Integer") || TypeString == TEXT("int") || TypeString == TEXT("int32") ||
        TypeString == TEXT("Float") || TypeString == TEXT("float") ||
        TypeString == TEXT("String") || TypeString == TEXT("FString") ||
        TypeString == TEXT("Vector") || TypeString == TEXT("FVector") ||
        TypeString == TEXT("Rotator") || TypeString == TEXT("FRotator"))
    {
        return reinterpret_cast<UObject*>(1); // Non-null placeholder for basic types
    }
    
    // Try to find as a class
    if (UClass* FoundClass = FindObject<UClass>(ANY_PACKAGE, *TypeString))
    {
        return FoundClass;
    }
    
    // Try to find as a struct
    if (UScriptStruct* FoundStruct = FindObject<UScriptStruct>(ANY_PACKAGE, *TypeString))
    {
        return FoundStruct;
    }
    
    // Try loading from common paths
    TArray<FString> SearchPaths = {
        FString::Printf(TEXT("/Script/Engine.%s"), *TypeString),
        FString::Printf(TEXT("/Script/CoreUObject.%s"), *TypeString),
        FString::Printf(TEXT("/Game/Blueprints/%s"), *TypeString)
    };
    
    for (const FString& SearchPath : SearchPaths)
    {
        if (UClass* LoadedClass = LoadClass<UObject>(nullptr, *SearchPath))
        {
            return LoadedClass;
        }
        
        if (UScriptStruct* LoadedStruct = LoadObject<UScriptStruct>(nullptr, *SearchPath))
        {
            return LoadedStruct;
        }
    }
    
    return nullptr;
}

