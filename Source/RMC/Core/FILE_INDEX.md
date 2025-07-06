# Decoupled Architecture File Index

This document provides a comprehensive index of all files related to the decoupled architecture implementation in the Platcom Framework. It serves as a reference for developers to quickly locate relevant files and understand their purpose.

## Table of Contents

1. [Core Systems](#core-systems)
2. [Interfaces](#interfaces)
3. [Example Components](#example-components)
4. [Documentation](#documentation)
5. [Updated Components](#updated-components)

## Core Systems

### GameEventSystem

- **GameEventSystem.h**: Defines the GameEventSystem class, event types, and event data structure
- **GameEventSystem.cpp**: Implements the GameEventSystem functionality

**Purpose**: Provides a centralized system for event-based communication between components without direct references.

**Key Features**:
- Event broadcasting and listening
- Event data structure for passing information
- Event listener registration and cleanup

### ComponentLocator

- **ComponentLocator.h**: Defines the ComponentLocator class and related types
- **ComponentLocator.cpp**: Implements the ComponentLocator functionality

**Purpose**: Allows components to find each other without direct references.

**Key Features**:
- Component registration and unregistration
- Component lookup by name or class
- Support for interface discovery

## Interfaces

### MomentumInterface

- **MomentumInterface.h**: Defines the IMomentumInterface interface

**Purpose**: Defines a contract for accessing and modifying momentum functionality.

**Key Methods**:
- `GetCurrentMomentum`: Returns the current momentum value
- `GetMomentumTier`: Returns the current momentum tier
- `AddMomentum`: Adds momentum from a specific source

### RiftInterface

- **RiftInterface.h**: Defines the IRiftInterface interface

**Purpose**: Defines a contract for accessing and controlling rift functionality.

**Key Methods**:
- `CanRift`: Checks if rifting is currently allowed
- `GetRiftCapabilities`: Returns the current rift capabilities
- `PerformPhantomDodge`: Performs a phantom dodge in a specified direction

### StyleInterface

- **StyleInterface.h**: Defines the IStyleInterface interface

**Purpose**: Defines a contract for accessing and modifying style points.

**Key Methods**:
- `GetCurrentStylePoints`: Returns the current style points
- `GetCurrentStyleRank`: Returns the current style rank
- `AddStylePoints`: Adds style points from a specific move

### WallRunInterface

- **WallRunInterface.h**: Defines the IWallRunInterface interface

**Purpose**: Defines a contract for accessing and controlling wall run functionality.

**Key Methods**:
- `CanWallRun`: Checks if wall running is currently allowed
- `IsWallRunning`: Checks if the character is currently wall running
- `TryStartWallRun`: Attempts to start wall running

## Example Components

### MomentumEffectsComponent

- **MomentumEffectsComponent.h**: Defines the MomentumEffectsComponent class
- **MomentumEffectsComponent.cpp**: Implements the MomentumEffectsComponent functionality

**Purpose**: Demonstrates how to use the event system to respond to momentum changes without direct references to the MomentumComponent.

**Key Features**:
- Event listening for momentum changes
- Visual and audio effects based on momentum
- Example of a component that works without direct references

### MomentumEffectsExample

- **MomentumEffectsExample.h**: Defines the MomentumEffectsExample class
- **MomentumEffectsExample.cpp**: Implements the MomentumEffectsExample functionality

**Purpose**: Demonstrates how to broadcast events to simulate momentum changes for testing purposes.

**Key Features**:
- Event broadcasting for momentum changes
- Methods to simulate different momentum scenarios
- Example of a component that works without direct references

## Documentation

### README.md

**Purpose**: Provides an overview of the decoupled architecture and serves as the main entry point for developers.

**Key Sections**:
- Core concepts
- Key components
- Getting started
- Documentation
- Implementation status
- Benefits

### QUICK_START.md

**Purpose**: Provides a quick introduction to using the decoupled architecture in components.

**Key Sections**:
- Core concepts
- Creating a new component
- Using interfaces
- Using the event system
- Using the component locator
- Common patterns
- Troubleshooting

### ARCHITECTURE_DIAGRAM.md

**Purpose**: Provides visual representations of the decoupled architecture to help developers understand the structure and flow of the system.

**Key Sections**:
- High-level architecture
- Component communication flow
- Interface hierarchy
- Event system architecture
- Component locator pattern
- Before vs. after comparison

### DEVELOPER_CHECKLIST.md

**Purpose**: Provides a step-by-step guide for implementing the decoupled architecture in components.

**Key Sections**:
- Creating a new component
- Updating an existing component
- Creating a new interface
- Testing your implementation
- Code review checklist

### IMPLEMENTATION_PLAN.md

**Purpose**: Outlines the phased approach for implementing the decoupled architecture across the entire codebase.

**Key Sections**:
- Overview
- Phase 1: Foundation
- Phase 2: Core Components
- Phase 3: Character Integration
- Phase 4: Blueprint Integration
- Phase 5: Performance Optimization
- Phase 6: Legacy Code Refactoring
- Phase 7: Documentation and Training
- Phase 8: Ongoing Support and Evolution
- Risk management
- Success metrics

### BENEFITS_SUMMARY.md

**Purpose**: Summarizes the benefits of the decoupled architecture for both technical and non-technical stakeholders.

**Key Sections**:
- Executive summary
- Technical benefits
- Business benefits
- Measuring success
- Comparison with previous architecture
- Team testimonials

### PATTERNS_AND_ANTIPATTERNS.md

**Purpose**: Documents common patterns and antipatterns when using the decoupled architecture.

**Key Sections**:
- Patterns (Interface Discovery, Event Broadcasting, etc.)
- Antipatterns (Direct Component Reference, Polling, etc.)
- Examples of good and bad practices
- Solutions to common problems

### GLOSSARY.md

**Purpose**: Defines key terms and concepts used in the decoupled architecture.

**Key Sections**:
- Alphabetical listing of terms
- Definitions and explanations
- Cross-references to related terms
- Examples where appropriate

### EXECUTIVE_SUMMARY.md

**Purpose**: Provides a high-level overview of the decoupled architecture for non-technical stakeholders.

**Key Sections**:
- Overview
- The problem
- The solution
- Business benefits
- Impact on development
- Success metrics
- Implementation plan
- Conclusion
- Testimonials

## Updated Components

### MomentumComponent

- **MomentumComponent.h**: Defines the MomentumComponent class
- **MomentumComponent.cpp**: Implements the MomentumComponent functionality

**Purpose**: Manages player momentum that affects abilities and damage.

**Key Updates**:
- Implements IMomentumInterface
- Uses event system for communication
- Registers with component locator
- Removes direct references to other components

### RiftComponent

- **RiftComponent.h**: Defines the RiftComponent class
- **RiftComponent.cpp**: Implements the RiftComponent functionality

**Purpose**: Manages teleportation/dash abilities.

**Key Updates**:
- Implements IRiftInterface
- Uses event system for communication
- Registers with component locator
- Removes direct references to other components

### StyleComponent

- **StyleComponent.h**: Defines the StyleComponent class
- **StyleComponent.cpp**: Implements the StyleComponent functionality

**Purpose**: Manages the style ranking system.

**Key Updates**:
- Implements IStyleInterface
- Uses event system for communication
- Registers with component locator
- Removes direct references to other components

### WallRunComponent

- **WallRunComponent.h**: Defines the WallRunComponent class
- **WallRunComponent.cpp**: Implements the WallRunComponent functionality

**Purpose**: Handles wall running mechanics.

**Key Updates**:
- Implements IWallRunInterface
- Uses event system for communication
- Registers with component locator
- Removes direct references to other components

### WeaponManagerComponent

- **WeaponManagerComponent.h**: Defines the WeaponManagerComponent class
- **WeaponManagerComponent.cpp**: Implements the WeaponManagerComponent functionality

**Purpose**: Manages weapon inventory and switching.

**Key Updates**:
- Uses interfaces for component interaction
- Uses event system for communication
- Registers with component locator
- Removes direct references to other components

### RMCCharacter

- **RMCCharacter.h**: Defines the RMCCharacter class
- **RMCCharacter.cpp**: Implements the RMCCharacter functionality

**Purpose**: The main player character class.

**Key Updates**:
- Adds ComponentLocator
- Uses interfaces for component interaction
- Uses event system for communication
- Updates input handling to broadcast events
- Removes direct references to components

## Conclusion

This file index provides a comprehensive overview of all files related to the decoupled architecture implementation in the Platcom Framework. It serves as a reference for developers to quickly locate relevant files and understand their purpose.

If you need to find a specific file or understand how different parts of the architecture work together, this index should be your first stop. For more detailed information about specific aspects of the architecture, refer to the documentation files listed above.