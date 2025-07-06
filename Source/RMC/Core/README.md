# Platcom Framework Decoupled Architecture

## Overview

The Platcom Framework Decoupled Architecture is a software design approach that reduces coupling between components, making the codebase more maintainable, extensible, and robust. This architecture enables components to communicate without direct references, using interfaces and an event system instead.

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Key Components](#key-components)
3. [Getting Started](#getting-started)
4. [Documentation](#documentation)
5. [Implementation Status](#implementation-status)
6. [Benefits](#benefits)
7. [Contributing](#contributing)

## Core Concepts

The decoupled architecture is built around three core concepts:

### 1. Interfaces

Interfaces define contracts for component functionality, allowing components to interact without direct references. For example, the `IMomentumInterface` defines methods for accessing and modifying momentum, which can be implemented by any component that provides momentum functionality.

```cpp
UINTERFACE(MinimalAPI, Blueprintable)
class UMomentumInterface : public UInterface
{
    GENERATED_BODY()
};

class IMomentumInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    float GetCurrentMomentum() const;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    int32 GetMomentumTier() const;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
    void AddMomentum(float Amount, FName SourceName);
};
```

### 2. Event System

The event system enables communication between components without direct references. Components can broadcast events and listen for events from other components, creating a loosely coupled system.

```cpp
// Broadcasting an event
FGameEventData EventData;
EventData.EventType = EGameEventType::MomentumChanged;
EventData.Instigator = GetOwner();
EventData.FloatValue = NewMomentum;
EventData.FloatParam1 = MomentumDelta;
EventSystem->BroadcastEvent(EventData);

// Listening for an event
EventListenerHandles.Add(EventSystem->AddEventListener(this, EGameEventType::MomentumChanged, 
    FName("OnMomentumChanged")));

// Event handler
void UMyComponent::OnMomentumChanged(const FGameEventData& EventData)
{
    float NewMomentum = EventData.FloatValue;
    float MomentumDelta = EventData.FloatParam1;
    // Handle the event
}
```

### 3. Component Locator

The component locator allows components to find each other without direct references. Components register themselves with the locator, and other components can find them by name or class.

```cpp
// Registering a component
UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
if (Locator)
{
    Locator->RegisterComponent(this, TEXT("MyComponent"));
}

// Finding a component
UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
if (Locator)
{
    UMomentumComponent* MomentumComp = Locator->GetComponentByClass<UMomentumComponent>();
    if (MomentumComp)
    {
        // Use the component
    }
}
```

## Key Components

The decoupled architecture includes the following key components:

### 1. GameEventSystem

A GameInstanceSubsystem that manages event broadcasting and listening. It allows components to communicate without direct references.

### 2. ComponentLocator

An ActorComponent that allows components to find each other without direct references. Components register themselves with the locator, and other components can find them by name or class.

### 3. Core Interfaces

- **IMomentumInterface**: Defines methods for accessing and modifying momentum.
- **IRiftInterface**: Defines methods for accessing and controlling rift functionality.
- **IStyleInterface**: Defines methods for accessing and modifying style points.
- **IWallRunInterface**: Defines methods for accessing and controlling wall run functionality.

### 4. Example Components

- **MomentumEffectsComponent**: Demonstrates how to use the event system to respond to momentum changes.
- **MomentumEffectsExample**: Demonstrates how to broadcast events to simulate momentum changes.

## Getting Started

To start using the decoupled architecture, follow these steps:

1. Read the [Quick Start Guide](QUICK_START.md) to understand the basics.
2. Review the [Architecture Diagrams](ARCHITECTURE_DIAGRAM.md) to understand the structure.
3. Follow the [Developer Checklist](DEVELOPER_CHECKLIST.md) when implementing or updating components.
4. Refer to the [Implementation Plan](IMPLEMENTATION_PLAN.md) for the overall roadmap.

## Documentation

The following documentation is available:

### Core Documentation

- [Quick Start Guide](QUICK_START.md): A quick introduction to using the architecture.
- [Architecture Diagrams](ARCHITECTURE_DIAGRAM.md): Visual representations of the architecture.
- [Developer Checklist](DEVELOPER_CHECKLIST.md): Step-by-step guide for implementing the architecture.
- [Implementation Plan](IMPLEMENTATION_PLAN.md): Roadmap for implementing the architecture across the codebase.

### Additional Documentation

- [Benefits Summary](BENEFITS_SUMMARY.md): Summary of the benefits of the architecture.
- [Patterns and Antipatterns](PATTERNS_AND_ANTIPATTERNS.md): Common patterns and antipatterns.
- [Glossary](GLOSSARY.md): Definitions of key terms and concepts.

## Implementation Status

The decoupled architecture is being implemented in phases. The current status is:

- **Phase 1 (Foundation)**: Completed
  - Core systems (GameEventSystem, ComponentLocator)
  - Core interfaces (MomentumInterface, RiftInterface, StyleInterface, WallRunInterface)
  - Example components
  - Documentation

- **Phase 2 (Core Components)**: In Progress
  - Updating MomentumComponent to implement IMomentumInterface
  - Updating RiftComponent to implement IRiftInterface
  - Updating StyleComponent to implement IStyleInterface
  - Updating WallRunComponent to implement IWallRunInterface
  - Updating WeaponManagerComponent to use interfaces and events

See the [Implementation Plan](IMPLEMENTATION_PLAN.md) for more details on the remaining phases.

## Benefits

The decoupled architecture provides several key benefits:

### Technical Benefits

1. **Reduced Coupling**: Components communicate through interfaces and events rather than direct references.
2. **Improved Testability**: Components can be tested in isolation without needing to instantiate their dependencies.
3. **Enhanced Extensibility**: New components can be added without modifying existing ones.
4. **Better Error Handling**: Components gracefully handle missing dependencies.
5. **Clearer Component Boundaries**: Interfaces clearly define what functionality components provide.

### Business Benefits

1. **Faster Development**: Reduced coupling leads to faster development cycles.
2. **Reduced Bug Count**: Clearer component boundaries lead to fewer bugs.
3. **Easier Onboarding**: New developers can understand and work on individual components without needing to comprehend the entire system.
4. **Improved Code Reusability**: Components with clear interfaces are more reusable.
5. **Better Scalability**: The architecture scales better as the project grows.

See the [Benefits Summary](BENEFITS_SUMMARY.md) for more details.

## Contributing

When contributing to the codebase, please follow these guidelines:

1. **Follow the Architecture**: Use interfaces and events for component communication.
2. **Avoid Direct References**: Don't create direct references between components.
3. **Use the Component Locator**: Register your components with the locator and use it to find other components.
4. **Broadcast Events**: Use the event system to broadcast state changes.
5. **Listen for Events**: Register for events from other components instead of polling them.
6. **Implement Interfaces**: If your component provides functionality that others might need, consider creating an interface for it.
7. **Write Tests**: Create unit tests for your components that test them in isolation.

For more detailed guidelines, see the [Developer Checklist](DEVELOPER_CHECKLIST.md).