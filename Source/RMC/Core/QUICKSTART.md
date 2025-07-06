# Platcom Framework Decoupling Quick Start Guide

## Introduction

This guide provides a quick overview of how to use the new decoupled architecture in the Platcom Framework. It covers the basic patterns and provides examples for common use cases.

## Using the Game Event System

### Step 1: Get the Event Subsystem

```cpp
// In BeginPlay or similar initialization function
UGameInstance* GameInstance = GetWorld()->GetGameInstance();
if (GameInstance)
{
    EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
}
```

### Step 2: Register for Events

```cpp
// Register for an event with a member function
int32 Handle = EventSystem->AddEventListenerWithObject(
    EGameEventType::MomentumChanged, 
    this, 
    FName("OnMomentumChanged")
);

// Store the handle for later cleanup
EventListenerHandles.Add(Handle);

// Or register with a dynamic delegate
FGameEventDelegate Delegate;
Delegate.BindDynamic(this, &UMyComponent::OnMomentumChanged);
Handle = EventSystem->AddEventListener(EGameEventType::MomentumChanged, Delegate);
EventListenerHandles.Add(Handle);
```

### Step 3: Implement Event Handler

```cpp
// Event handler function
UFUNCTION()
void UMyComponent::OnMomentumChanged(const FGameEventData& EventData)
{
    // Access event data
    float NewMomentum = EventData.FloatValue;
    bool bGained = EventData.BoolValue;
    AActor* Instigator = EventData.Instigator;
    
    // Handle the event
    // ...
}
```

### Step 4: Broadcast Events

```cpp
// Create event data
FGameEventData EventData;
EventData.EventType = EGameEventType::MomentumChanged;
EventData.Instigator = GetOwner();
EventData.FloatValue = CurrentMomentum;
EventData.BoolValue = (MomentumDelta > 0.0f);

// Broadcast the event
EventSystem->BroadcastEvent(EventData);
```

### Step 5: Clean Up

```cpp
// In EndPlay or similar cleanup function
for (int32 Handle : EventListenerHandles)
{
    EventSystem->RemoveEventListener(Handle);
}
EventListenerHandles.Empty();
```

## Using Component Interfaces

### Step 1: Implement an Interface

```cpp
// In your component header
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMyComponent : public UActorComponent, public IMomentumInterface
{
    GENERATED_BODY()
    
    // IMomentumInterface implementation
    virtual float GetCurrentMomentum_Implementation() const override;
    virtual int32 GetMomentumTier_Implementation() const override;
    // ... other interface methods
};
```

### Step 2: Implement Interface Methods

```cpp
// In your component implementation
float UMyComponent::GetCurrentMomentum_Implementation() const
{
    return CurrentMomentum;
}

int32 UMyComponent::GetMomentumTier_Implementation() const
{
    // Implementation logic
    return CurrentTier;
}
```

### Step 3: Use the Interface

```cpp
// Check if an actor implements the interface
if (Actor->Implements<UMomentumInterface>())
{
    // Get momentum value through the interface
    float Momentum = IMomentumInterface::Execute_GetCurrentMomentum(Actor);
    
    // Check if an action is allowed
    bool bCanPerform = IMomentumInterface::Execute_CanPerformMomentumAction(
        Actor, 
        FName("SpecialMove"), 
        50.0f
    );
}
```

## Using the Component Locator

### Step 1: Add the Component Locator to Your Actor

```cpp
// In your actor's constructor
ComponentLocator = CreateDefaultSubobject<UComponentLocator>(TEXT("ComponentLocator"));
```

### Step 2: Register Components

```cpp
// In your component's OnRegister
void UMyComponent::OnRegister()
{
    Super::OnRegister();
    
    // Register with the component locator if available
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
        if (Locator)
        {
            Locator->RegisterComponent(this, FName("MyComponent"));
        }
    }
}
```

### Step 3: Find Components

```cpp
// Find by name
UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
if (Locator)
{
    UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(
        Locator->GetComponentByName(FName("MomentumComponent"))
    );
    
    // Or find by class
    UMomentumComponent* MomentumComp2 = Locator->GetComponentByClass<UMomentumComponent>();
}
```

### Step 4: Unregister Components

```cpp
// In your component's OnUnregister
void UMyComponent::OnUnregister()
{
    // Unregister from the component locator if available
    AActor* Owner = GetOwner();
    if (Owner)
    {
        UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
        if (Locator)
        {
            Locator->UnregisterComponent(this);
        }
    }
    
    Super::OnUnregister();
}
```

## Common Use Cases

### Responding to Momentum Changes

```cpp
// Register for momentum changed events
int32 Handle = EventSystem->AddEventListenerWithObject(
    EGameEventType::MomentumChanged, 
    this, 
    FName("OnMomentumChanged")
);

// Event handler
void UMyComponent::OnMomentumChanged(const FGameEventData& EventData)
{
    float NewMomentum = EventData.FloatValue;
    bool bGained = EventData.BoolValue;
    
    // Update visuals, gameplay mechanics, etc.
}
```

### Checking if an Action is Allowed

```cpp
// Check if the owner has enough momentum for an action
bool UMyComponent::CanPerformAction(FName ActionName, float RequiredMomentum)
{
    AActor* Owner = GetOwner();
    if (Owner && Owner->Implements<UMomentumInterface>())
    {
        return IMomentumInterface::Execute_CanPerformMomentumAction(
            Owner, 
            ActionName, 
            RequiredMomentum
        );
    }
    
    return false;
}
```

### Adding Style Points

```cpp
// Broadcast style points gained event
void UMyComponent::AddStylePoints(float Points, FName MoveName)
{
    FGameEventData EventData;
    EventData.EventType = EGameEventType::StylePointsGained;
    EventData.Instigator = GetOwner();
    EventData.FloatValue = Points;
    EventData.NameValue = MoveName;
    
    EventSystem->BroadcastEvent(EventData);
}
```

## Best Practices

1. **Always Clean Up Event Listeners**: Unregister event listeners in EndPlay to prevent memory leaks.

2. **Use Interfaces for Direct Queries**: Use interfaces when you need immediate responses, and events for notifications.

3. **Keep Event Data Meaningful**: Include all relevant information in event data to minimize the need for additional queries.

4. **Check for Null References**: Always check if subsystems and components are valid before using them.

5. **Use Descriptive Event Types**: Create specific event types for different kinds of events to make the code more maintainable.

6. **Consider Performance**: Events are great for decoupling, but excessive event broadcasting can impact performance.

## Troubleshooting

### Event Handlers Not Being Called

- Ensure the event type matches exactly
- Verify that the function name in AddEventListenerWithObject matches your handler function
- Check that the handler function has the correct signature (takes a FGameEventData parameter)
- Make sure the event system is properly initialized

### Interface Methods Not Working

- Ensure the interface is properly implemented with _Implementation suffix
- Check that the Execute_ call is using the correct interface method
- Verify that the actor actually implements the interface with Actor->Implements<UInterface>()

### Component Locator Not Finding Components

- Ensure components are registered with the locator
- Check that the service name or class type matches exactly
- Verify that the component locator is added to the actor