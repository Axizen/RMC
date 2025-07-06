# Patterns and Antipatterns in the Decoupled Architecture

This document outlines common patterns and antipatterns when using the decoupled architecture in the Platcom Framework. Following these guidelines will help you create components that are maintainable, extensible, and robust.

## Table of Contents

1. [Patterns](#patterns)
   - [Interface Discovery Pattern](#interface-discovery-pattern)
   - [Event Broadcasting Pattern](#event-broadcasting-pattern)
   - [Event Listening Pattern](#event-listening-pattern)
   - [Component Registration Pattern](#component-registration-pattern)
   - [Graceful Degradation Pattern](#graceful-degradation-pattern)
   - [Interface Adapter Pattern](#interface-adapter-pattern)
   - [Event Chaining Pattern](#event-chaining-pattern)
   - [Lazy Initialization Pattern](#lazy-initialization-pattern)
   - [Component Composition Pattern](#component-composition-pattern)
   - [State Change Notification Pattern](#state-change-notification-pattern)

2. [Antipatterns](#antipatterns)
   - [Direct Component Reference Antipattern](#direct-component-reference-antipattern)
   - [Polling Antipattern](#polling-antipattern)
   - [God Component Antipattern](#god-component-antipattern)
   - [Event Spam Antipattern](#event-spam-antipattern)
   - [Tight Coupling Antipattern](#tight-coupling-antipattern)
   - [Missing Cleanup Antipattern](#missing-cleanup-antipattern)
   - [Interface Bloat Antipattern](#interface-bloat-antipattern)
   - [Premature Optimization Antipattern](#premature-optimization-antipattern)
   - [Inconsistent Event Handling Antipattern](#inconsistent-event-handling-antipattern)
   - [Circular Dependency Antipattern](#circular-dependency-antipattern)

## Patterns

### Interface Discovery Pattern

**Description:** A pattern for finding interfaces implemented by other components without direct references.

**Implementation:**

```cpp
IMomentumInterface* UMyComponent::GetMomentumInterface() const
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

**Benefits:**
- Eliminates direct references to specific component types
- Allows for flexible component composition
- Supports multiple implementation strategies (component-based or actor-based)

### Event Broadcasting Pattern

**Description:** A pattern for broadcasting events to notify other components of state changes or actions.

**Implementation:**

```cpp
void UMomentumComponent::BroadcastMomentumChangedEvent(float NewMomentum, float MomentumDelta)
{
    if (EventSystem)
    {
        FGameEventData EventData;
        EventData.EventType = EGameEventType::MomentumChanged;
        EventData.Instigator = GetOwner();
        EventData.FloatValue = NewMomentum;
        EventData.FloatParam1 = MomentumDelta;
        
        EventSystem->BroadcastEvent(EventData);
    }
    
    // Also broadcast through delegate for backward compatibility
    OnMomentumChanged.Broadcast(NewMomentum, MomentumDelta);
}
```

**Benefits:**
- Decouples event source from event handlers
- Allows multiple components to respond to events
- Provides a centralized mechanism for event distribution

### Event Listening Pattern

**Description:** A pattern for listening to events from other components without direct references.

**Implementation:**

```cpp
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
    UpdateVisualEffects(NewMomentum);
}
```

**Benefits:**
- Eliminates direct references to event sources
- Allows components to respond to events from multiple sources
- Provides a consistent mechanism for event handling

### Component Registration Pattern

**Description:** A pattern for registering components with the ComponentLocator to make them discoverable by other components.

**Implementation:**

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
```

**Benefits:**
- Makes components discoverable without direct references
- Supports dynamic component addition and removal
- Provides a centralized registry for component discovery

### Graceful Degradation Pattern

**Description:** A pattern for gracefully handling missing dependencies or failed operations.

**Implementation:**

```cpp
void UMyComponent::PerformAction()
{
    // Try to get the momentum interface
    IMomentumInterface* MomentumInterface = GetMomentumInterface();
    if (MomentumInterface)
    {
        // Use the interface
        IMomentumInterface::Execute_AddMomentum(MomentumInterface->_getUObject(), 10.0f, FName("Action"));
    }
    else
    {
        // Gracefully handle missing interface
        UE_LOG(LogGame, Warning, TEXT("MomentumInterface not found. Using default behavior."));
        
        // Implement fallback behavior
        PlayDefaultEffect();
    }
}
```

**Benefits:**
- Prevents crashes due to missing dependencies
- Provides fallback behavior when dependencies are unavailable
- Makes components more robust in different configurations

### Interface Adapter Pattern

**Description:** A pattern for adapting between different interfaces or providing a simplified interface to complex functionality.

**Implementation:**

```cpp
// Adapter component that provides a simplified interface to the momentum system
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMomentumAdapterComponent : public UActorComponent, public IMomentumInterface
{
    GENERATED_BODY()

public:
    // Simplified method for blueprints
    UFUNCTION(BlueprintCallable, Category = "Momentum")
    void AddMomentumForAction(FName ActionName);
    
    // Interface implementation
    virtual float GetCurrentMomentum_Implementation() const override;
    virtual int32 GetMomentumTier_Implementation() const override;
    virtual void AddMomentum_Implementation(float Amount, FName SourceName) override;

protected:
    // Helper method to get the actual momentum component
    UMomentumComponent* GetMomentumComponent() const;
};

// Implementation
void UMomentumAdapterComponent::AddMomentumForAction(FName ActionName)
{
    // Map action names to momentum values
    float MomentumAmount = 0.0f;
    if (ActionName == FName("Jump"))
    {
        MomentumAmount = 5.0f;
    }
    else if (ActionName == FName("Dash"))
    {
        MomentumAmount = 10.0f;
    }
    else if (ActionName == FName("Attack"))
    {
        MomentumAmount = 15.0f;
    }
    
    // Add momentum
    AddMomentum_Implementation(MomentumAmount, ActionName);
}

float UMomentumAdapterComponent::GetCurrentMomentum_Implementation() const
{
    UMomentumComponent* MomentumComp = GetMomentumComponent();
    if (MomentumComp)
    {
        return MomentumComp->GetCurrentMomentum_Implementation();
    }
    return 0.0f;
}
```

**Benefits:**
- Simplifies complex interfaces for specific use cases
- Provides a layer of abstraction between components
- Makes interfaces more accessible to Blueprint developers

### Event Chaining Pattern

**Description:** A pattern for chaining events together to create complex behaviors from simple components.

**Implementation:**

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

void UMyComponent::OnComboIncreased(const FGameEventData& EventData)
{
    // Update combo counter
    CurrentCombo = EventData.IntValue;
    
    // Chain another event if combo threshold reached
    if (CurrentCombo >= ComboThreshold)
    {
        FGameEventData NewEventData;
        NewEventData.EventType = EGameEventType::ComboThresholdReached;
        NewEventData.Instigator = GetOwner();
        NewEventData.IntValue = CurrentCombo;
        
        EventSystem->BroadcastEvent(NewEventData);
    }
}
```

**Benefits:**
- Creates complex behaviors from simple components
- Allows for flexible behavior composition
- Supports emergent gameplay features

### Lazy Initialization Pattern

**Description:** A pattern for initializing resources only when they are needed, rather than at component creation.

**Implementation:**

```cpp
IMomentumInterface* UMyComponent::GetMomentumInterface()
{
    // Lazy initialization of cached interface
    if (!CachedMomentumInterface)
    {
        UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
        if (Locator)
        {
            UActorComponent* Component = Locator->GetComponentByClass<UMomentumComponent>();
            if (Component && Component->Implements<UMomentumInterface>())
            {
                CachedMomentumInterface = Cast<IMomentumInterface>(Component);
            }
        }
    }
    
    return CachedMomentumInterface;
}
```

**Benefits:**
- Reduces initialization overhead
- Improves startup performance
- Only allocates resources when needed

### Component Composition Pattern

**Description:** A pattern for building complex behaviors by composing multiple simple components.

**Implementation:**

```cpp
// In the actor's constructor
AMyActor::AMyActor()
{
    // Create the component locator first
    ComponentLocator = CreateDefaultSubobject<UComponentLocator>(TEXT("ComponentLocator"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create gameplay components
    MomentumComponent = CreateDefaultSubobject<UMomentumComponent>(TEXT("MomentumComponent"));
    StyleComponent = CreateDefaultSubobject<UStyleComponent>(TEXT("StyleComponent"));
    RiftComponent = CreateDefaultSubobject<URiftComponent>(TEXT("RiftComponent"));
    
    // Create effect components
    MomentumEffectsComponent = CreateDefaultSubobject<UMomentumEffectsComponent>(TEXT("MomentumEffectsComponent"));
    StyleEffectsComponent = CreateDefaultSubobject<UStyleEffectsComponent>(TEXT("StyleEffectsComponent"));
    RiftEffectsComponent = CreateDefaultSubobject<URiftEffectsComponent>(TEXT("RiftEffectsComponent"));
}
```

**Benefits:**
- Creates complex behaviors from simple components
- Improves code reusability
- Makes the system more modular and maintainable

### State Change Notification Pattern

**Description:** A pattern for notifying other components of state changes through events.

**Implementation:**

```cpp
void UMomentumComponent::UpdateMomentumTier()
{
    // Calculate new tier
    int32 NewTier = CalculateMomentumTier();
    
    // Check if tier changed
    if (NewTier != CurrentMomentumTier)
    {
        // Update tier
        int32 OldTier = CurrentMomentumTier;
        CurrentMomentumTier = NewTier;
        
        // Broadcast tier changed event
        if (EventSystem)
        {
            FGameEventData EventData;
            EventData.EventType = EGameEventType::MomentumTierChanged;
            EventData.Instigator = GetOwner();
            EventData.IntValue = NewTier;
            EventData.IntParam1 = OldTier;
            
            EventSystem->BroadcastEvent(EventData);
        }
        
        // Also broadcast through delegate for backward compatibility
        OnMomentumTierChanged.Broadcast(NewTier);
    }
}
```

**Benefits:**
- Decouples state management from state visualization
- Allows multiple components to respond to state changes
- Provides a consistent mechanism for state change notification

## Antipatterns

### Direct Component Reference Antipattern

**Description:** Directly referencing other components by type, creating tight coupling between components.

**Example:**

```cpp
// Bad: Direct component reference
UMomentumComponent* MomentumComponent;

void UMyComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Direct lookup creates tight coupling
    MomentumComponent = GetOwner()->FindComponentByClass<UMomentumComponent>();
}

void UMyComponent::DoSomething()
{
    // Direct call to another component
    if (MomentumComponent)
    {
        MomentumComponent->AddMomentum(10.0f, FName("Action"));
    }
}
```

**Problems:**
- Creates tight coupling between components
- Makes components harder to test in isolation
- Reduces code reusability
- Makes the system less flexible and harder to extend

**Solution:**
- Use interfaces instead of direct component references
- Use the event system for communication between components
- Use the component locator for component discovery

### Polling Antipattern

**Description:** Repeatedly checking the state of another component instead of responding to events.

**Example:**

```cpp
// Bad: Polling for state changes
void UMyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Poll for momentum changes
    IMomentumInterface* MomentumInterface = GetMomentumInterface();
    if (MomentumInterface)
    {
        float CurrentMomentum = IMomentumInterface::Execute_GetCurrentMomentum(MomentumInterface->_getUObject());
        
        // Check if momentum changed
        if (CurrentMomentum != LastMomentum)
        {
            // Handle momentum change
            UpdateVisualEffects(CurrentMomentum);
            LastMomentum = CurrentMomentum;
        }
    }
}
```

**Problems:**
- Wastes CPU cycles checking for changes that might not happen
- Creates unnecessary dependencies between components
- Makes the system less responsive to changes
- Can lead to race conditions and inconsistent state

**Solution:**
- Register for events instead of polling
- Use the event system to receive notifications of state changes
- Only update when necessary

### God Component Antipattern

**Description:** Creating a component that does too many things or knows too much about other components.

**Example:**

```cpp
// Bad: God component that does too many things
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UGameplayManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Too many responsibilities
    void UpdateMomentum(float Amount, FName SourceName);
    void UpdateStyle(float Points, FName MoveName);
    void PerformRift(FVector Location);
    void StartWallRun();
    void EndWallRun();
    void FireWeapon();
    void ReloadWeapon();
    void SwitchWeapon(int32 Index);
    // And many more...

protected:
    // Direct references to many components
    UPROPERTY()
    UMomentumComponent* MomentumComponent;
    
    UPROPERTY()
    UStyleComponent* StyleComponent;
    
    UPROPERTY()
    URiftComponent* RiftComponent;
    
    UPROPERTY()
    UWallRunComponent* WallRunComponent;
    
    UPROPERTY()
    UWeaponManagerComponent* WeaponManager;
};
```

**Problems:**
- Creates a central point of failure
- Makes the component hard to understand and maintain
- Creates tight coupling between many components
- Violates the single responsibility principle

**Solution:**
- Split the component into smaller, focused components
- Use interfaces and events for communication
- Follow the single responsibility principle

### Event Spam Antipattern

**Description:** Broadcasting too many events or events that are too fine-grained, leading to performance issues and complexity.

**Example:**

```cpp
// Bad: Event spam
void UMomentumComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Decay momentum
    float OldMomentum = CurrentMomentum;
    CurrentMomentum = FMath::Max(CurrentMomentum - (MomentumDecayRate * DeltaTime), 0.0f);
    
    // Broadcast event on every tick, even for tiny changes
    if (!FMath::IsNearlyEqual(CurrentMomentum, OldMomentum))
    {
        BroadcastMomentumChangedEvent(CurrentMomentum, CurrentMomentum - OldMomentum);
    }
}
```

**Problems:**
- Creates performance overhead from excessive event processing
- Makes the system harder to debug
- Can lead to event loops and cascading updates
- Increases complexity for event handlers

**Solution:**
- Only broadcast events for significant changes
- Use throttling or debouncing for frequent changes
- Consider batching related events
- Use appropriate event granularity

### Tight Coupling Antipattern

**Description:** Creating components that are tightly coupled to specific implementations rather than interfaces.

**Example:**

```cpp
// Bad: Tight coupling to specific implementation
void UMyComponent::DoSomething()
{
    // Directly accessing another component's internal state
    UMomentumComponent* MomentumComp = GetOwner()->FindComponentByClass<UMomentumComponent>();
    if (MomentumComp)
    {
        // Accessing internal state directly
        if (MomentumComp->CurrentMomentum >= 50.0f && MomentumComp->bIsOvercharged)
        {
            // Do something
        }
    }
}
```

**Problems:**
- Creates dependencies on specific implementations
- Makes components harder to test in isolation
- Reduces code reusability
- Makes the system less flexible and harder to extend

**Solution:**
- Use interfaces instead of specific implementations
- Access state through interface methods
- Use events for state change notification

### Missing Cleanup Antipattern

**Description:** Failing to clean up resources, event listeners, or registrations when a component is destroyed.

**Example:**

```cpp
// Bad: Missing cleanup
void UMyComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register for events
    EventSystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameEventSubsystem>();
    if (EventSystem)
    {
        EventSystem->AddEventListener(this, EGameEventType::MomentumChanged, 
            FName("OnMomentumChanged"));
    }
    
    // No corresponding cleanup in EndPlay
}
```

**Problems:**
- Can lead to memory leaks
- Can cause crashes when events are delivered to destroyed components
- Makes the system less stable
- Can cause hard-to-debug issues

**Solution:**
- Always clean up resources in EndPlay
- Store event listener handles and remove them in EndPlay
- Unregister from the component locator in OnUnregister

### Interface Bloat Antipattern

**Description:** Creating interfaces with too many methods or methods that are too specific to particular implementations.

**Example:**

```cpp
// Bad: Interface bloat
UINTERFACE(MinimalAPI, Blueprintable)
class UMomentumInterface : public UInterface
{
    GENERATED_BODY()
};

class IMomentumInterface
{
    GENERATED_BODY()

public:
    // Too many methods, some too specific
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    float GetCurrentMomentum() const;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    int32 GetMomentumTier() const;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    void AddMomentum(float Amount, FName SourceName);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    void DecayMomentum(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    bool SpendMomentum(float Amount);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    void BankMomentum(float Amount);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    float WithdrawBankedMomentum(float Amount);
    
    // And many more implementation-specific methods...
};
```

**Problems:**
- Makes interfaces harder to implement
- Creates unnecessary dependencies on implementation details
- Reduces the flexibility of the interface
- Makes the system harder to extend

**Solution:**
- Keep interfaces focused and minimal
- Only include methods that are truly needed by clients
- Use multiple interfaces for different aspects of functionality
- Consider using the Interface Segregation Principle

### Premature Optimization Antipattern

**Description:** Optimizing code before it's necessary, leading to increased complexity and potential bugs.

**Example:**

```cpp
// Bad: Premature optimization
IMomentumInterface* UMyComponent::GetMomentumInterface()
{
    // Complex caching logic that might not be necessary
    static TWeakObjectPtr<UObject> CachedImplementer;
    static float CacheTime = 0.0f;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (!CachedImplementer.IsValid() || CurrentTime - CacheTime > 0.1f)
    {
        UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
        if (Locator)
        {
            UActorComponent* Component = Locator->GetComponentByClass<UMomentumComponent>();
            if (Component && Component->Implements<UMomentumInterface>())
            {
                CachedImplementer = Component;
                CacheTime = CurrentTime;
            }
        }
    }
    
    return CachedImplementer.IsValid() ? Cast<IMomentumInterface>(CachedImplementer.Get()) : nullptr;
}
```

**Problems:**
- Increases code complexity
- Makes the code harder to understand and maintain
- Can introduce subtle bugs
- May not actually improve performance

**Solution:**
- Start with simple, clear code
- Measure performance before optimizing
- Only optimize when necessary
- Focus on algorithmic improvements rather than micro-optimizations

### Inconsistent Event Handling Antipattern

**Description:** Handling the same event differently in different components, leading to inconsistent behavior.

**Example:**

```cpp
// Bad: Inconsistent event handling
// In Component A
void UComponentA::OnMomentumChanged(const FGameEventData& EventData)
{
    // Assumes FloatValue is the new momentum
    float NewMomentum = EventData.FloatValue;
    // Doesn't use momentum delta
    
    // Update visuals based on new momentum
    UpdateVisuals(NewMomentum);
}

// In Component B
void UComponentB::OnMomentumChanged(const FGameEventData& EventData)
{
    // Assumes FloatParam1 is the momentum delta
    float MomentumDelta = EventData.FloatParam1;
    // Doesn't use new momentum
    
    // Play effects based on momentum delta
    PlayEffects(MomentumDelta);
}
```

**Problems:**
- Creates inconsistent behavior across components
- Makes the system harder to understand and debug
- Can lead to subtle bugs
- Makes the system less maintainable

**Solution:**
- Document event data structure clearly
- Use consistent event handling patterns
- Consider creating helper functions for common event handling
- Use typed events when possible

### Circular Dependency Antipattern

**Description:** Creating circular dependencies between components, where each component depends on the other.

**Example:**

```cpp
// Bad: Circular dependency
// In MomentumComponent.h
#include "StyleComponent.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UMomentumComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Direct reference to StyleComponent
    UPROPERTY()
    UStyleComponent* StyleComponent;
    
    void AddMomentum(float Amount, FName SourceName);
};

// In StyleComponent.h
#include "MomentumComponent.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UStyleComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Direct reference to MomentumComponent
    UPROPERTY()
    UMomentumComponent* MomentumComponent;
    
    void AddStylePoints(float Points, FName MoveName);
};
```

**Problems:**
- Creates compilation issues
- Makes the system harder to understand
- Creates tight coupling between components
- Makes components impossible to use independently

**Solution:**
- Use interfaces instead of direct references
- Use the event system for communication
- Use forward declarations when necessary
- Redesign component responsibilities to eliminate circular dependencies

## Conclusion

Following these patterns and avoiding these antipatterns will help you create components that are maintainable, extensible, and robust. The decoupled architecture is designed to promote good practices and discourage bad ones, but it's up to you as a developer to use it effectively.

Remember that the goal is to reduce coupling between components, making the codebase more maintainable and extensible. By using interfaces, events, and the component locator, you can create components that communicate without direct references, leading to a more robust and flexible system.