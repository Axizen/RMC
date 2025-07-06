# Decoupled Component Implementation Checklist

This checklist provides a step-by-step guide for implementing new components or refactoring existing ones to use the decoupled architecture in the Platcom Framework.

## Creating a New Component

### 1. Component Design

- [ ] Define the component's responsibilities and functionality
- [ ] Identify which interfaces the component should implement
- [ ] Determine which events the component should broadcast
- [ ] Determine which events the component should listen for
- [ ] Identify any dependencies on other components

### 2. Header File Setup

- [ ] Create the component header file
- [ ] Include necessary interface headers
- [ ] Declare the component class with appropriate interfaces
- [ ] Declare interface implementation methods with `_Implementation` suffix
- [ ] Declare event handler methods
- [ ] Declare properties for storing event listener handles
- [ ] Declare methods for registering and unregistering event listeners

### 3. Implementation File Setup

- [ ] Create the component implementation file
- [ ] Implement the component constructor with default values
- [ ] Implement `BeginPlay` to initialize the component and register for events
- [ ] Implement `EndPlay` to clean up resources and unregister from events
- [ ] Implement interface methods with `_Implementation` suffix
- [ ] Implement event handler methods
- [ ] Implement methods for registering and unregistering event listeners

### 4. Event System Integration

- [ ] Get the event subsystem in `BeginPlay`
- [ ] Register for relevant events
- [ ] Implement event handler methods
- [ ] Broadcast events when appropriate
- [ ] Unregister from events in `EndPlay`

### 5. Component Locator Integration

- [ ] Override `OnRegister` to register with the component locator
- [ ] Override `OnUnregister` to unregister from the component locator
- [ ] Use the component locator to find other components when needed

### 6. Testing

- [ ] Test the component in isolation
- [ ] Test the component's interaction with other components
- [ ] Test edge cases and error handling
- [ ] Verify that events are being broadcast and handled correctly

## Refactoring an Existing Component

### 1. Component Analysis

- [ ] Identify direct references to other components
- [ ] Identify methods that could be replaced with events
- [ ] Identify functionality that should be exposed through interfaces
- [ ] Determine which interfaces the component should implement

### 2. Header File Updates

- [ ] Include necessary interface headers
- [ ] Add interface implementations to the component class declaration
- [ ] Declare interface implementation methods with `_Implementation` suffix
- [ ] Declare event handler methods
- [ ] Declare properties for storing event listener handles
- [ ] Declare methods for registering and unregistering event listeners

### 3. Implementation File Updates

- [ ] Update the component constructor if needed
- [ ] Update `BeginPlay` to initialize the component and register for events
- [ ] Update `EndPlay` to clean up resources and unregister from events
- [ ] Implement interface methods with `_Implementation` suffix
- [ ] Implement event handler methods
- [ ] Implement methods for registering and unregistering event listeners
- [ ] Replace direct component references with interface calls or event system

### 4. Event System Integration

- [ ] Get the event subsystem in `BeginPlay`
- [ ] Register for relevant events
- [ ] Implement event handler methods
- [ ] Broadcast events when appropriate
- [ ] Unregister from events in `EndPlay`

### 5. Component Locator Integration

- [ ] Override `OnRegister` to register with the component locator
- [ ] Override `OnUnregister` to unregister from the component locator
- [ ] Replace direct component lookups with component locator

### 6. Backward Compatibility

- [ ] Maintain existing public methods and properties
- [ ] Update existing methods to use the new architecture internally
- [ ] Add deprecation notices to methods that should be phased out

### 7. Testing

- [ ] Test the refactored component to ensure it still works as expected
- [ ] Test the component's interaction with other components
- [ ] Test edge cases and error handling
- [ ] Verify that events are being broadcast and handled correctly

## Example: Event Registration and Handling

```cpp
// In BeginPlay
void UMyComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the event subsystem
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    if (GameInstance)
    {
        EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
        if (EventSystem)
        {
            RegisterForEvents();
        }
    }
}

// Register for events
void UMyComponent::RegisterForEvents()
{
    if (!EventSystem)
    {
        return;
    }
    
    // Register for momentum changed events
    int32 Handle = EventSystem->AddEventListenerWithObject(
        EGameEventType::MomentumChanged, 
        this, 
        FName("OnMomentumChanged")
    );
    EventListenerHandles.Add(Handle);
    
    // Register for other events as needed
}

// Event handler
UFUNCTION()
void UMyComponent::OnMomentumChanged(const FGameEventData& EventData)
{
    // Handle the event
    float NewMomentum = EventData.FloatValue;
    bool bGained = EventData.BoolValue;
    
    // Update component state based on the event
}

// In EndPlay
void UMyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from events
    if (EventSystem)
    {
        for (int32 Handle : EventListenerHandles)
        {
            EventSystem->RemoveEventListener(Handle);
        }
        EventListenerHandles.Empty();
    }
    
    Super::EndPlay(EndPlayReason);
}
```

## Example: Interface Implementation

```cpp
// In header file
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMyComponent : public UActorComponent, public IMomentumInterface
{
    GENERATED_BODY()
    
    // IMomentumInterface implementation
    virtual float GetCurrentMomentum_Implementation() const override;
    virtual int32 GetMomentumTier_Implementation() const override;
    // ... other interface methods
};

// In implementation file
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

## Example: Component Locator Integration

```cpp
// In OnRegister
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

// In OnUnregister
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

## Example: Broadcasting Events

```cpp
// Broadcasting an event
void UMyComponent::NotifyStateChanged(bool bNewState)
{
    if (!EventSystem)
    {
        return;
    }
    
    // Create event data
    FGameEventData EventData;
    EventData.EventType = EGameEventType::CustomStateChanged;
    EventData.Instigator = GetOwner();
    EventData.BoolValue = bNewState;
    
    // Broadcast the event
    EventSystem->BroadcastEvent(EventData);
}
```

By following this checklist, you can ensure that your components are properly integrated with the decoupled architecture of the Platcom Framework, making your code more maintainable, extensible, and robust.