# Decoupled Architecture Glossary

This glossary defines key terms and concepts used in the decoupled architecture of the Platcom Framework. It serves as a reference for developers to understand the terminology used throughout the documentation.

## A

### Actor Component
A reusable module that can be added to an Actor to extend its functionality. In Unreal Engine, Actor Components are represented by the `UActorComponent` class and its derivatives.

### Adapter Pattern
A design pattern that allows incompatible interfaces to work together by creating a wrapper that translates one interface to another. In the decoupled architecture, adapter components can be used to provide simplified interfaces to complex systems.

## B

### Blueprint
A visual scripting system in Unreal Engine that allows designers to create gameplay elements without writing code. The decoupled architecture provides Blueprint-friendly interfaces and events to make it accessible to Blueprint developers.

### Broadcasting
The act of sending an event to all registered listeners. In the decoupled architecture, components broadcast events through the GameEventSystem to notify other components of state changes or actions.

## C

### Component
See Actor Component.

### Component Locator
A service that allows components to find each other without direct references. In the decoupled architecture, the ComponentLocator is an ActorComponent that maintains a registry of components that can be looked up by name or class.

### Coupling
The degree of interdependence between software modules. High coupling means that changes in one module may affect other modules, while low coupling means that modules are more independent. The decoupled architecture aims to reduce coupling between components.

## D

### Decoupled Architecture
A software design approach that reduces coupling between components, making the codebase more maintainable, extensible, and robust. In the Platcom Framework, the decoupled architecture uses interfaces, events, and the component locator to achieve this goal.

### Delegate
A type-safe function pointer in Unreal Engine that allows objects to register callbacks for specific events. In the decoupled architecture, delegates are used for backward compatibility alongside the event system.

### Dependency Injection
A design pattern where dependencies are provided to an object rather than created by the object itself. In the decoupled architecture, dependencies are often injected through interfaces or the component locator.

## E

### Event
A notification that something has happened, such as a state change or an action. In the decoupled architecture, events are represented by the FGameEventData struct and are broadcast through the GameEventSystem.

### Event Handler
A function that is called when a specific event occurs. In the decoupled architecture, event handlers are methods marked with the UFUNCTION() macro that take an FGameEventData parameter.

### Event Listener
An object that registers to receive notifications when specific events occur. In the decoupled architecture, components register as event listeners through the GameEventSystem.

### Event System
A system that allows components to communicate without direct references by broadcasting and listening for events. In the decoupled architecture, the GameEventSystem is a GameInstanceSubsystem that manages event broadcasting and listening.

## F

### FGameEventData
A struct that contains data about an event, including the event type, instigator, target, and various parameters. In the decoupled architecture, this struct is used to pass information between components through the event system.

## G

### GameEventSystem
A GameInstanceSubsystem that manages event broadcasting and listening. In the decoupled architecture, this system allows components to communicate without direct references.

### GameInstanceSubsystem
A type of subsystem in Unreal Engine that exists for the lifetime of the game instance. In the decoupled architecture, the GameEventSystem is implemented as a GameInstanceSubsystem to ensure it's available throughout the game.

## I

### Interface
A contract that defines a set of methods that a class must implement. In the decoupled architecture, interfaces like IMomentumInterface define contracts for component functionality, allowing components to interact without direct references.

### Interface Segregation Principle
A design principle that states that clients should not be forced to depend on interfaces they do not use. In the decoupled architecture, interfaces are kept focused and minimal to adhere to this principle.

## L

### Listener
See Event Listener.

### Loose Coupling
A design goal where components have minimal knowledge of each other. In the decoupled architecture, loose coupling is achieved through interfaces, events, and the component locator.

## M

### MomentumComponent
A component that manages player momentum, affecting abilities and damage. In the decoupled architecture, this component implements the IMomentumInterface and uses the event system to communicate with other components.

### MomentumInterface
An interface that defines methods for accessing and modifying momentum. In the decoupled architecture, this interface allows components to interact with the momentum system without direct references to the MomentumComponent.

## O

### Observer Pattern
A design pattern where an object (the subject) maintains a list of dependents (observers) and notifies them of state changes. In the decoupled architecture, the event system implements a variation of this pattern.

## P

### Pattern
A reusable solution to a common problem in software design. In the decoupled architecture, patterns like the Interface Discovery Pattern and Event Broadcasting Pattern provide guidance for implementing components.

### Polling
Repeatedly checking the state of another component instead of responding to events. In the decoupled architecture, polling is discouraged in favor of event-based communication.

## R

### RiftComponent
A component that manages teleportation/dash abilities. In the decoupled architecture, this component implements the IRiftInterface and uses the event system to communicate with other components.

### RiftInterface
An interface that defines methods for accessing and controlling rift functionality. In the decoupled architecture, this interface allows components to interact with the rift system without direct references to the RiftComponent.

## S

### Service Locator Pattern
A design pattern that provides a central registry of services that can be looked up by name or type. In the decoupled architecture, the ComponentLocator implements this pattern for component discovery.

### Single Responsibility Principle
A design principle that states that a class should have only one reason to change. In the decoupled architecture, components are designed to have focused responsibilities to adhere to this principle.

### StyleComponent
A component that manages the style ranking system. In the decoupled architecture, this component implements the IStyleInterface and uses the event system to communicate with other components.

### StyleInterface
An interface that defines methods for accessing and modifying style points. In the decoupled architecture, this interface allows components to interact with the style system without direct references to the StyleComponent.

### Subsystem
A module in Unreal Engine that provides functionality across multiple objects. In the decoupled architecture, the GameEventSystem is implemented as a GameInstanceSubsystem.

## T

### Tight Coupling
A design anti-pattern where components have detailed knowledge of each other, creating dependencies that make the system harder to maintain and extend. The decoupled architecture aims to eliminate tight coupling between components.

## U

### UFUNCTION
A macro in Unreal Engine that marks a function for use with the reflection system. In the decoupled architecture, event handler methods must be marked with the UFUNCTION() macro to be callable by the event system.

### UINTERFACE
A macro in Unreal Engine that defines an interface class. In the decoupled architecture, interfaces like UMomentumInterface are defined using this macro.

## W

### WallRunComponent
A component that handles wall running mechanics. In the decoupled architecture, this component implements the IWallRunInterface and uses the event system to communicate with other components.

### WallRunInterface
An interface that defines methods for accessing and controlling wall run functionality. In the decoupled architecture, this interface allows components to interact with the wall run system without direct references to the WallRunComponent.

### WeaponManagerComponent
A component that manages weapon inventory and switching. In the decoupled architecture, this component uses interfaces and the event system to communicate with other components.

## Conclusion

This glossary provides definitions for key terms and concepts used in the decoupled architecture of the Platcom Framework. It serves as a reference for developers to understand the terminology used throughout the documentation.

If you encounter a term that is not defined in this glossary, please refer to the [Unreal Engine Documentation](https://docs.unrealengine.com) or contact the architecture team for clarification.