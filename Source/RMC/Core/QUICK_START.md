# Decoupled Architecture Quick Start Guide

This guide provides a quick introduction to using the decoupled architecture in the Platcom Framework. It covers the basics of creating components that use interfaces and the event system for communication.

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Creating a New Component](#creating-a-new-component)
3. [Using Interfaces](#using-interfaces)
4. [Using the Event System](#using-the-event-system)
5. [Using the Component Locator](#using-the-component-locator)
6. [Common Patterns](#common-patterns)
7. [Troubleshooting](#troubleshooting)

## Core Concepts

The decoupled architecture is built around three core concepts:

1. **Interfaces**: Define contracts for component functionality
2. **Event System**: Enables communication between components without direct references
3. **Component Locator**: Allows components to find each other without direct references

By using these concepts, components can communicate without direct dependencies, making the codebase more maintainable, extensible, and robust.

## Creating a New Component

Here's how to create a new component that follows the decoupled architecture:

### 1. Create the Component Header

```cpp
// MyComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/GameEventSystem.h"
#include "MyComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMyComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    
    // Called when the component is registered
    virtual void OnRegister() override;
    
    // Called when the component is unregistered
    virtual void OnUnregister() override;
    
    // Called when the component is destroyed
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    // Event system reference
    UPROPERTY()
    UGameEventSubsystem* EventSystem;
    
    // Event listener handles
    TArray<FEventListenerHandle> EventListenerHandles;
    
    // Event handlers
    UFUNCTION()
    void OnMomentumChanged(const FGameEventData& EventData);
    
    UFUNCTION()
    void OnStylePointsGained(const FGameEventData& EventData);
};
```

### 2. Implement the Component

```cpp
// MyComponent.cpp
#include "MyComponent.h"
#include "Core/ComponentLocator.h"
#include "Core/MomentumInterface.h"

UMyComponent::UMyComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UMyComponent::OnRegister()
{
    Super::OnRegister();
    
    // Register with the component locator
    UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
    if (Locator)
    {
        Locator->RegisterComponent(this, TEXT("MyComponent"));
    }
}

void UMyComponent::OnUnregister()
{
    // Unregister from the component locator
    UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
    if (Locator)
    {
        Locator->UnregisterComponent(TEXT("MyComponent"));
    }
    
    Super::OnUnregister();
}

void UMyComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the event system
    EventSystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameEventSubsystem>();
    if (EventSystem)
    {
        // Register for events
        EventListenerHandles.Add(EventSystem->AddEventListener(this, EGameEventType::MomentumChanged, 
            FName("OnMomentumChanged")));
        EventListenerHandles.Add(EventSystem->AddEventListener(this, EGameEventType::StylePointsGained, 
            FName("OnStylePointsGained")));
    }
}

void UMyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up event listeners
    if (EventSystem)
    {
        for (const FEventListenerHandle& Handle : EventListenerHandles)
        {
            EventSystem->RemoveEventListener(Handle);
        }
        EventListenerHandles.Empty();
    }
    
    Super::EndPlay(EndPlayReason);
}

void UMyComponent::OnMomentumChanged(const FGameEventData& EventData)
{
    // Handle momentum changed event
    float NewMomentum = EventData.FloatValue;
    float MomentumDelta = EventData.FloatParam1;
    
    // Do something with the momentum values
    UE_LOG(LogTemp, Log, TEXT("Momentum changed to %f (delta: %f)"), NewMomentum, MomentumDelta);
}

void UMyComponent::OnStylePointsGained(const FGameEventData& EventData)
{
    // Handle style points gained event
    float Points = EventData.FloatValue;
    FName MoveName = EventData.NameValue;
    
    // Do something with the style points
    UE_LOG(LogTemp, Log, TEXT("Style points gained: %f from move: %s"), Points, *MoveName.ToString());
}
```

## Using Interfaces

Interfaces define contracts for component functionality. Here's how to use them:

### 1. Finding an Interface

```cpp
IMomentumInterface* GetMomentumInterface()
{
    // Try to find through component locator first
    UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
    if (Locator)
    {
        UActorComponent* Component = Locator->GetComponentByClass<UMomentumComponent>();
        if (Component && Component->Implements<UMomentumInterface>())
        {
            return Cast<IMomentumInterface>(Component);
        }
    }
    
    // Fallback to direct owner check
    if (GetOwner()->Implements<UMomentumInterface>())
    {
        return Cast<IMomentumInterface>(GetOwner());
    }
    
    return nullptr;
}
```

### 2. Calling Interface Methods

```cpp
void UMyComponent::AddMomentum(float Amount, FName SourceName)
{
    IMomentumInterface* MomentumInterface = GetMomentumInterface();
    if (MomentumInterface)
    {
        IMomentumInterface::Execute_AddMomentum(MomentumInterface->_getUObject(), Amount, SourceName);
    }
}
```

## Using the Event System

The event system enables communication between components without direct references. Here's how to use it:

### 1. Broadcasting Events

```cpp
void UMyComponent::TriggerEffect()
{
    if (EventSystem)
    {
        // Create event data
        FGameEventData EventData;
        EventData.EventType = EGameEventType::EffectTriggered;
        EventData.Instigator = GetOwner();
        EventData.FloatValue = EffectStrength;
        EventData.NameValue = EffectName;
        
        // Broadcast the event
        EventSystem->BroadcastEvent(EventData);
    }
}
```

### 2. Listening for Events

```cpp
void UMyComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the event system
    EventSystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameEventSubsystem>();
    if (EventSystem)
    {
        // Register for events
        EventListenerHandles.Add(EventSystem->AddEventListener(this, EGameEventType::PlayerDamaged, 
            FName("OnPlayerDamaged")));
    }
}

// Event handler
void UMyComponent::OnPlayerDamaged(const FGameEventData& EventData)
{
    // Handle player damaged event
    float DamageAmount = EventData.FloatValue;
    AActor* DamageCauser = EventData.Target;
    
    // Do something with the damage information
    PlayDamageEffect(DamageAmount, DamageCauser);
}
```

## Using the Component Locator

The Component Locator allows components to find each other without direct references. Here's how to use it:

### 1. Registering a Component

```cpp
void UMyComponent::OnRegister()
{
    Super::OnRegister();
    
    // Register with the component locator
    UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
    if (Locator)
    {
        Locator->RegisterComponent(this, TEXT("MyComponent"));
    }
}
```

### 2. Finding a Component

```cpp
UStyleComponent* FindStyleComponent()
{
    UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
    if (Locator)
    {
        return Locator->GetComponentByClass<UStyleComponent>();
    }
    return nullptr;
}
```

## Common Patterns

Here are some common patterns for using the decoupled architecture:

### 1. Responding to State Changes

```cpp
// Register for state change events
EventListenerHandles.Add(EventSystem->AddEventListener(this, EGameEventType::MomentumTierChanged, 
    FName("OnMomentumTierChanged")));

// Handle state change
void UMyComponent::OnMomentumTierChanged(const FGameEventData& EventData)
{
    int32 NewTier = EventData.IntValue;
    PlayTierEffect(NewTier);
}
```

### 2. Chaining Events

```cpp
void UMyComponent::OnEnemyDefeated(const FGameEventData& EventData)
{
    // Process the enemy defeated event
    AActor* DefeatedEnemy = EventData.Target;
    
    // Chain a new event
    FGameEventData NewEventData;
    NewEventData.EventType = EGameEventType::ComboIncreased;
    NewEventData.Instigator = GetOwner();
    NewEventData.IntValue = CurrentCombo + 1;
    
    EventSystem->BroadcastEvent(NewEventData);
}
```

### 3. Conditional Event Handling

```cpp
void UMyComponent::OnWeaponFired(const FGameEventData& EventData)
{
    // Only respond if this is our owner
    if (EventData.Instigator != GetOwner())
    {
        return;
    }
    
    // Handle the event
    PlayWeaponEffect();
}
```

## Troubleshooting

### Event Handlers Not Being Called

1. **Check Event Registration**: Make sure you're registering for the correct event type.
2. **Check Method Name**: The method name in AddEventListener must match the actual method name.
3. **Check Method Signature**: Event handler methods must have the signature `void Method(const FGameEventData&)`.
4. **Check UFUNCTION Macro**: Event handler methods must be marked with the UFUNCTION() macro.

### Interface Methods Not Working

1. **Check Implementation**: Make sure the component implements the interface correctly.
2. **Check Method Names**: Interface implementation methods must have the `_Implementation` suffix.
3. **Check Interface Cast**: Make sure you're casting to the correct interface type.

### Component Locator Not Finding Components

1. **Check Registration**: Make sure components are registering with the locator in OnRegister.
2. **Check Timing**: Components might not be registered yet if you're looking for them too early.
3. **Check Owner**: Make sure you're looking for components on the correct owner.

## Conclusion

This quick start guide covers the basics of using the decoupled architecture in the Platcom Framework. By following these patterns, you can create components that communicate without direct dependencies, making your code more maintainable, extensible, and robust.

For more detailed information, see the following resources:

- [Architecture Diagrams](ARCHITECTURE_DIAGRAM.md)
- [Benefits Summary](BENEFITS_SUMMARY.md)
- [Implementation Plan](IMPLEMENTATION_PLAN.md)
- [Developer Checklist](DEVELOPER_CHECKLIST.md)