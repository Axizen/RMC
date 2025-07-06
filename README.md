# Platcom Framework

## Overview

The Platcom Framework is a sophisticated movement and combat system for action games built with Unreal Engine 5.4. It features a decoupled architecture that makes the codebase more maintainable, extensible, and robust.

## Key Features

### Movement System

- **Momentum-Based Movement**: Dynamic movement system that rewards skillful play
- **Wall Running**: Smooth wall running mechanics with momentum integration
- **Rift System**: Teleportation/dash abilities with chaining mechanics

### Combat System

- **Weapon Management**: Flexible weapon system with multiple weapon types
- **Style Ranking**: DMC-inspired style ranking system that rewards creative play
- **Momentum Integration**: Momentum affects combat abilities and damage

### Progression System

- **Character Progression**: Character advancement system
- **Weapon Progression**: Weapons that level up with use

## Decoupled Architecture

The Platcom Framework uses a decoupled architecture that reduces dependencies between components through interfaces, events, and a component locator. This architecture provides several benefits:

- **Reduced Coupling**: Components communicate through events and interfaces rather than direct references
- **Improved Testability**: Components can be tested in isolation
- **Enhanced Extensibility**: New components can be added without modifying existing ones
- **Better Error Handling**: Components gracefully handle missing dependencies
- **Clearer Component Boundaries**: Interfaces clearly define what functionality components provide

For more information on the decoupled architecture, see the [Core README](Source/RMC/Core/README.md).

## Core Systems

### GameEventSystem

The GameEventSystem is a central hub for event-based communication between components. It allows components to broadcast events and listen for events without direct references to each other.

### ComponentLocator

The ComponentLocator is a service locator that allows components to find each other without direct references. Components register themselves with the locator, and other components can find them by name or interface.

### Component Interfaces

Interfaces define contracts for component interaction. They allow components to interact without direct references to each other.

## Key Components

### MomentumComponent

Manages the player's momentum, which affects abilities and damage. It implements the IMomentumInterface.

### RiftComponent

Manages teleportation/dash abilities. It implements the IRiftInterface.

### StyleComponent

Manages the player's style ranking, similar to Devil May Cry. It implements the IStyleInterface.

### WeaponManagerComponent

Manages the player's weapons and weapon switching.

### WallRunComponent

Manages wall running mechanics. It implements the IWallRunInterface.

## Getting Started

### Prerequisites

- Unreal Engine 5.4
- Visual Studio 2022 or later

### Installation

1. Clone the repository
2. Open the RMC.uproject file
3. Compile the project

### Documentation

Comprehensive documentation is available in the [Core directory](Source/RMC/Core/):

- [Quick Start Guide](Source/RMC/Core/QUICK_START.md)
- [Architecture Diagrams](Source/RMC/Core/ARCHITECTURE_DIAGRAM.md)
- [Developer Checklist](Source/RMC/Core/DEVELOPER_CHECKLIST.md)
- [Patterns and Antipatterns](Source/RMC/Core/PATTERNS_AND_ANTIPATTERNS.md)
- [Benefits Summary](Source/RMC/Core/BENEFITS_SUMMARY.md)
- [Implementation Plan](Source/RMC/Core/IMPLEMENTATION_PLAN.md)
- [Glossary](Source/RMC/Core/GLOSSARY.md)

## Contributing

When contributing to the Platcom Framework, please follow the decoupled architecture patterns and avoid the antipatterns. See [Patterns and Antipatterns](Source/RMC/Core/PATTERNS_AND_ANTIPATTERNS.md) for details.

## License

This code is proprietary and confidential. Unauthorized copying, transfer, or reproduction of the contents of this repository is prohibited.