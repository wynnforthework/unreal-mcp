#include "Factories/ComponentFactory.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/MeshComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
// #include "Components/ParticleSystemComponent.h" // Not available in UE 5.6
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
// #include "Components/MovementComponent.h" // Not available in UE 5.6
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/Engine.h"

FComponentFactory& FComponentFactory::Get()
{
    static FComponentFactory Instance;
    return Instance;
}

void FComponentFactory::RegisterComponentType(const FString& TypeName, UClass* ComponentClass)
{
    if (!ComponentClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("FComponentFactory::RegisterComponentType: Attempted to register null ComponentClass for type '%s'"), *TypeName);
        return;
    }

    if (!ComponentClass->IsChildOf(UActorComponent::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("FComponentFactory::RegisterComponentType: Class '%s' is not a valid ActorComponent"), *ComponentClass->GetName());
        return;
    }

    FScopeLock Lock(&ComponentMapLock);
    ComponentTypeMap.Add(TypeName, ComponentClass);
    
    UE_LOG(LogTemp, Log, TEXT("FComponentFactory: Registered component type '%s' -> '%s'"), *TypeName, *ComponentClass->GetName());
}

UClass* FComponentFactory::GetComponentClass(const FString& TypeName) const
{
    FScopeLock Lock(&ComponentMapLock);
    
    // Ensure default types are initialized
    if (!bDefaultTypesInitialized)
    {
        // Cast away const to initialize - this is safe as we're using a lock
        const_cast<FComponentFactory*>(this)->InitializeDefaultTypes();
    }

    if (UClass* const* FoundClass = ComponentTypeMap.Find(TypeName))
    {
        return *FoundClass;
    }

    UE_LOG(LogTemp, Warning, TEXT("FComponentFactory::GetComponentClass: Component type '%s' not found"), *TypeName);
    return nullptr;
}

TArray<FString> FComponentFactory::GetAvailableTypes() const
{
    FScopeLock Lock(&ComponentMapLock);
    
    // Ensure default types are initialized
    if (!bDefaultTypesInitialized)
    {
        // Cast away const to initialize - this is safe as we're using a lock
        const_cast<FComponentFactory*>(this)->InitializeDefaultTypes();
    }

    TArray<FString> AvailableTypes;
    ComponentTypeMap.GetKeys(AvailableTypes);
    
    // Sort alphabetically for consistent output
    AvailableTypes.Sort();
    
    return AvailableTypes;
}

void FComponentFactory::InitializeDefaultTypes()
{
    FScopeLock Lock(&ComponentMapLock);
    
    if (bDefaultTypesInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("FComponentFactory: Initializing default component types"));

    // Scene Components
    RegisterComponentType(TEXT("SceneComponent"), USceneComponent::StaticClass());
    
    // Mesh Components
    RegisterComponentType(TEXT("StaticMeshComponent"), UStaticMeshComponent::StaticClass());
    RegisterComponentType(TEXT("SkeletalMeshComponent"), USkeletalMeshComponent::StaticClass());
    
    // Light Components
    RegisterComponentType(TEXT("PointLightComponent"), UPointLightComponent::StaticClass());
    RegisterComponentType(TEXT("SpotLightComponent"), USpotLightComponent::StaticClass());
    RegisterComponentType(TEXT("DirectionalLightComponent"), UDirectionalLightComponent::StaticClass());
    
    // Collision Components
    RegisterComponentType(TEXT("SphereComponent"), USphereComponent::StaticClass());
    RegisterComponentType(TEXT("BoxComponent"), UBoxComponent::StaticClass());
    RegisterComponentType(TEXT("CapsuleComponent"), UCapsuleComponent::StaticClass());
    
    // Audio Components
    RegisterComponentType(TEXT("AudioComponent"), UAudioComponent::StaticClass());
    
    // Particle Components (UParticleSystemComponent not available in UE 5.6)
    // RegisterComponentType(TEXT("ParticleSystemComponent"), UParticleSystemComponent::StaticClass());
    
    // Camera Components
    RegisterComponentType(TEXT("CameraComponent"), UCameraComponent::StaticClass());
    
    // Movement Components
    RegisterComponentType(TEXT("CharacterMovementComponent"), UCharacterMovementComponent::StaticClass());
    RegisterComponentType(TEXT("FloatingPawnMovement"), UFloatingPawnMovement::StaticClass());
    RegisterComponentType(TEXT("ProjectileMovementComponent"), UProjectileMovementComponent::StaticClass());
    RegisterComponentType(TEXT("RotatingMovementComponent"), URotatingMovementComponent::StaticClass());
    
    // UI Components
    RegisterComponentType(TEXT("WidgetComponent"), UWidgetComponent::StaticClass());
    
    // Other Components
    RegisterComponentType(TEXT("DecalComponent"), UDecalComponent::StaticClass());
    RegisterComponentType(TEXT("SplineComponent"), USplineComponent::StaticClass());
    RegisterComponentType(TEXT("TimelineComponent"), UTimelineComponent::StaticClass());
    RegisterComponentType(TEXT("InputComponent"), UInputComponent::StaticClass());

    bDefaultTypesInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("FComponentFactory: Initialized %d default component types"), ComponentTypeMap.Num());
}