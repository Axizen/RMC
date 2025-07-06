# Platcom Framework Decoupling Implementation

## Overview

This document summarizes the changes made to reduce coupling between components in the Platcom Framework. The goal was to create a more maintainable, extensible, and robust architecture while preserving all existing functionality.

## Key Changes

### 1. Created a Game Event System

Implemented a centralized event system that allows components to communicate without direct references to each other.

**Files Created:**
- `Source/RMC/Core/GameEventSystem.h`
- `Source/RMC/Core/GameEventSystem.cpp`

**Key Features:**
- Event broadcasting with rich data payload
- Event listening with both dynamic delegates and object callbacks
- Support for multiple listeners per event type
- Automatic cleanup of listeners when objects are destroyed

### 2. Implemented Component Interfaces

Created interfaces for key components, allowing other systems to interact with them without direct references.

**Files Created:**
- `Source/RMC/Core/MomentumInterface.h`
- `Source/RMC/Core/RiftInterface.h`
- `Source/RMC/Core/StyleInterface.h`

**Key Benefits:**
- Components can query functionality without knowing the concrete implementation
- Easier to swap implementations or create mock versions for testing
- Clearer contracts between components

### 3. Created a Component Locator

Implemented a service locator pattern to find components without direct references.

**Files Created:**
- `Source/RMC/Core/ComponentLocator.h`
- `Source/RMC/Core/ComponentLocator.cpp`

**Key Features:**
- Register components by name or class
- Find components by name or class
- Automatic registration and unregistration

### 4. Updated MomentumComponent

Refactored the MomentumComponent to implement the MomentumInterface and use the event system.

**Files Modified:**
- `Source/RMC/MomentumComponent.h`
- `Source/RMC/MomentumComponent.cpp`

**Key Changes:**
- Implemented MomentumInterface
- Added event broadcasting through the event system
- Maintained backward compatibility with existing delegates
- Added registration with ComponentLocator

### 5. Created Example Effect Component

Implemented a MomentumEffectsComponent that demonstrates how to use the event system to respond to momentum events without direct references.

**Files Created:**
- `Source/RMC/Core/MomentumEffectsComponent.h`
- `Source/RMC/Core/MomentumEffectsComponent.cpp`

**Key Features:**
- Listens for momentum events through the event system
- Plays visual and audio effects based on momentum changes
- Completely decoupled from MomentumComponent

## Benefits of the New Architecture

### 1. Reduced Coupling

Components no longer need direct references to each other, making the codebase more maintainable and easier to refactor.

### 2. Improved Testability

Interfaces and the event system make it easier to create mock implementations for testing.

### 3. Enhanced Extensibility

New components can be added without modifying existing ones, as long as they follow the established patterns.

### 4. Better Debugging

The event system provides a central place to monitor and debug communication between components.

### 5. Clearer Component Boundaries

Interfaces define clear contracts between components, making it easier to understand how they interact.

## Implementation Example

Here's an example of how the new architecture works:

1. **MomentumComponent** changes momentum and broadcasts an event through the **GameEventSystem**:

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

2. **MomentumEffectsComponent** listens for the event and responds accordingly:

```cpp
// Register for momentum changed events
int32 Handle = EventSystem->AddEventListenerWithObject(
    EGameEventType::MomentumChanged, 
    this, 
    FName("OnMomentumChanged")
);

// Event handler
void UMomentumEffectsComponent::OnMomentumChanged(const FGameEventData& EventData)
{
    // Handle momentum change
    bool bGained = EventData.BoolValue;
    PlayMomentumChangeEffect(bGained);
}
```

3. Other components can query momentum through the **MomentumInterface**:

```cpp
// Check if actor implements the interface
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

## Next Steps

To complete the decoupling of the Platcom Framework, the following steps are recommended:

1. Update the remaining components (RiftComponent, StyleComponent, etc.) to implement their respective interfaces and use the event system.

2. Refactor any direct component references in the codebase to use interfaces or the event system instead.

3. Add the ComponentLocator to the main character or game mode to make it easily accessible.

4. Create a configuration system to make gameplay parameters more data-driven.

5. Add comprehensive logging to the event system to aid in debugging.

## Conclusion

The changes made to the Platcom Framework have significantly reduced coupling between components while preserving all existing functionality. The new architecture is more maintainable, extensible, and robust, making it easier to add new features and fix bugs in the future.