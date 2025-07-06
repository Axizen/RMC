# Platcom Framework Architecture Diagrams

This document provides visual representations of the Platcom Framework's decoupled architecture to help developers understand the structure and flow of the system.

## Table of Contents

1. [High-Level Architecture](#high-level-architecture)
2. [Component Communication Flow](#component-communication-flow)
3. [Interface Hierarchy](#interface-hierarchy)
4. [Event System Architecture](#event-system-architecture)
5. [Component Locator Pattern](#component-locator-pattern)
6. [Before vs. After Comparison](#before-vs-after-comparison)

## High-Level Architecture

```
+---------------------------------------------+
|                RMCCharacter                 |
+---------------------------------------------+
|                                             |
|  +-------------+      +-----------------+   |
|  | Component   |<---->| GameEventSystem |   |
|  | Locator     |      +-----------------+   |
|  +-------------+            ^               |
|        ^                    |               |
|        |                    v               |
|  +-----v------+      +-----------------+    |
|  | Component  |<---->| Component      |    |
|  | Interface  |      | Implementation |    |
|  +------------+      +-----------------+    |
|                                             |
+---------------------------------------------+
```

The high-level architecture shows how components interact through interfaces and the event system, with the ComponentLocator providing a way to discover components without direct references.

## Component Communication Flow

```
+----------------+    Event    +----------------+
| Component A    |----------->| Component B    |
| (Publisher)    |            | (Subscriber)   |
+----------------+            +----------------+
        |                            ^
        |                            |
        |        +----------------+  |
        +------->| Event System   |--+
                 +----------------+
```

```
+----------------+  Interface  +----------------+
| Component A    |----------->| Component B    |
| (Consumer)     |            | (Provider)     |
+----------------+            +----------------+
        |                            ^
        |                            |
        |        +----------------+  |
        +------->| Component     |--+
                 | Locator       |
                 +----------------+
```

These diagrams show the two primary ways components communicate in our architecture:
1. Through events published to the GameEventSystem
2. Through interfaces discovered via the ComponentLocator

## Interface Hierarchy

```
+-------------------+
| UInterface        |
+-------------------+
         ^
         |
+--------v----------+
| IMomentumInterface|
+-------------------+
         ^
         |
+--------v----------+
| UMomentumComponent|
+-------------------+
```

```
+-------------------+
| UInterface        |
+-------------------+
         ^
         |
+--------v----------+
| IRiftInterface    |
+-------------------+
         ^
         |
+--------v----------+
| URiftComponent    |
+-------------------+
```

```
+-------------------+
| UInterface        |
+-------------------+
         ^
         |
+--------v----------+
| IStyleInterface   |
+-------------------+
         ^
         |
+--------v----------+
| UStyleComponent   |
+-------------------+
```

```
+-------------------+
| UInterface        |
+-------------------+
         ^
         |
+--------v----------+
| IWallRunInterface |
+-------------------+
         ^
         |
+--------v----------+
| UWallRunComponent |
+-------------------+
```

These diagrams show how our component interfaces inherit from UInterface and are implemented by their respective components.

## Event System Architecture

```
+---------------------+
| GameEventSubsystem  |
+---------------------+
| - EventListeners    |
+---------------------+
| + BroadcastEvent()  |
| + AddEventListener()|
| + RemoveListener()  |
+---------------------+
          ^
          |
          v
+---------------------+
| FGameEventData      |
+---------------------+
| - EventType         |
| - Instigator        |
| - Target            |
| - FloatValue        |
| - IntValue          |
| - NameValue         |
| - LocationValue     |
+---------------------+
          ^
          |
          v
+---------------------+
| EGameEventType      |
+---------------------+
| - MomentumChanged   |
| - StylePointsGained |
| - WeaponFired       |
| - RiftPerformed     |
| - WallRunStarted    |
| - PlayerDamaged     |
| - ...               |
+---------------------+
```

This diagram shows the structure of our event system, including the GameEventSubsystem that manages events, the FGameEventData struct that contains event data, and the EGameEventType enum that defines all possible event types.

## Component Locator Pattern

```
+-------------------+
| ComponentLocator  |
+-------------------+
| - RegisteredComps |
| - ClassComponents |
+-------------------+
| + RegisterComp()  |
| + GetCompByName() |
| + GetCompByClass()|
+-------------------+
          ^
          |
    +-----+------+
    |            |
+---v----+  +---v----+
|Component|  |Component|
|    A    |  |    B    |
+---------+  +---------+
```

This diagram illustrates the ComponentLocator pattern, showing how components register themselves with the locator and how other components can find them through the locator.

## Before vs. After Comparison

### Before: Tightly Coupled Architecture

```
+---------------+       +---------------+
| RMCCharacter  |<----->| MomentumComp |
+---------------+       +---------------+
        |                      ^
        v                      |
+---------------+       +---------------+
| RiftComponent |<----->| StyleComponent|
+---------------+       +---------------+
        |                      ^
        v                      |
+---------------+       +---------------+
| WeaponManager |<----->| WallRunComp   |
+---------------+       +---------------+
```

### After: Decoupled Architecture

```
+---------------+
| RMCCharacter  |
+---------------+
        |
        v
+---------------+       +---------------+
| ComponentLoc  |<----->| EventSystem   |
+---------------+       +---------------+
        ^                      ^
        |                      |
+-------v-------+       +-----v---------+
| IMomentumInt  |       | IRiftInterface |
+---------------+       +---------------+
        ^                      ^
        |                      |
+-------v-------+       +-----v---------+
| MomentumComp  |       | RiftComponent |
+---------------+       +---------------+
```

These diagrams contrast the tightly coupled architecture before our changes with the decoupled architecture after. In the before diagram, components have direct references to each other, creating a web of dependencies. In the after diagram, components communicate through interfaces and the event system, eliminating direct dependencies.

## Sequence Diagram: Event Flow

```
+-------------+  +-------------+  +-------------+  +-------------+
| RMCCharacter|  | EventSystem |  | MomentumComp|  | StyleComp   |
+-------------+  +-------------+  +-------------+  +-------------+
      |                 |                |                |
      | Fire Weapon     |                |                |
      |---------------->|                |                |
      |                 | WeaponFired    |                |
      |                 |--------------->|                |
      |                 |                | Add Momentum   |
      |                 |                |--------------->|
      |                 |                |                | Add Style
      |                 |                |                |---------->
      |                 |                |                |
      |                 | MomentumChanged|                |
      |                 |<---------------|                |
      |                 | StyleChanged   |                |
      |                 |<-------------------------------|                
      |                 |                |                |
      | Update UI       |                |                |
      |<----------------|                |                |
      |                 |                |                |
```

This sequence diagram shows how events flow through the system when a player fires a weapon. The RMCCharacter broadcasts a WeaponFired event, which the MomentumComponent and StyleComponent respond to. They in turn broadcast MomentumChanged and StyleChanged events, which the RMCCharacter uses to update the UI.

## Conclusion

These diagrams provide a visual representation of our decoupled architecture, showing how components interact through interfaces and events rather than direct references. This architecture reduces coupling between components, making the codebase more maintainable, extensible, and robust.