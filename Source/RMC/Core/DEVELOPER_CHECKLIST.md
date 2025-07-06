# Developer Checklist for Implementing Decoupled Architecture

This checklist provides step-by-step guidance for implementing the decoupled architecture in your components. Follow these steps to ensure your components properly integrate with the architecture.

## Table of Contents

1. [Creating a New Component](#creating-a-new-component)
2. [Updating an Existing Component](#updating-an-existing-component)
3. [Creating a New Interface](#creating-a-new-interface)
4. [Testing Your Implementation](#testing-your-implementation)
5. [Code Review Checklist](#code-review-checklist)

## Creating a New Component

### Initial Setup

- [ ] Create header file with appropriate includes:
  ```cpp
  #include "CoreMinimal.h"
  #include "Components/ActorComponent.h"
  #include "Core/GameEventSystem.h"
  #include "YourComponent.generated.h"
  ```

- [ ] Define the component class with appropriate metadata:
  ```cpp
  UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
  class RMC_API UYourComponent : public UActorComponent
  ```

- [ ] Add necessary lifecycle methods:
  ```cpp
  virtual void BeginPlay() override;
  virtual void OnRegister() override;
  virtual void OnUnregister() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  ```

### Event System Integration

- [ ] Add event system reference:
  ```cpp
  UPROPERTY()
  UGameEventSubsystem* EventSystem;
  ```

- [ ] Add event listener handles array:
  ```cpp
  TArray<FEventListenerHandle> EventListenerHandles;
  ```

- [ ] Add event handler methods with UFUNCTION macro:
  ```cpp
  UFUNCTION()
  void OnSomeEvent(const FGameEventData& EventData);
  ```

- [ ] Initialize event system in BeginPlay:
  ```cpp
  EventSystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameEventSubsystem>();
  ```

- [ ] Register for events in BeginPlay:
  ```cpp
  if (EventSystem)
  {
      EventListenerHandles.Add(EventSystem->AddEventListener(this, EGameEventType::SomeEvent, 
          FName("OnSomeEvent")));
  }
  ```

- [ ] Clean up event listeners in EndPlay:
  ```cpp
  if (EventSystem)
  {
      for (const FEventListenerHandle& Handle : EventListenerHandles)
      {
          EventSystem->RemoveEventListener(Handle);
      }
      EventListenerHandles.Empty();
  }
  ```

### Component Locator Integration

- [ ] Register with component locator in OnRegister:
  ```cpp
  UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
  if (Locator)
  {
      Locator->RegisterComponent(this, TEXT("YourComponent"));
  }
  ```

- [ ] Unregister from component locator in OnUnregister:
  ```cpp
  UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
  if (Locator)
  {
      Locator->UnregisterComponent(TEXT("YourComponent"));
  }
  ```

### Interface Usage

- [ ] Add helper methods to find interfaces:
  ```cpp
  IMomentumInterface* GetMomentumInterface() const;
  IRiftInterface* GetRiftInterface() const;
  ```

- [ ] Implement helper methods:
  ```cpp
  IMomentumInterface* UYourComponent::GetMomentumInterface() const
  {
      UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
      if (Locator)
      {
          UActorComponent* Component = Locator->GetComponentByClass<UMomentumComponent>();
          if (Component && Component->Implements<UMomentumInterface>())
          {
              return Cast<IMomentumInterface>(Component);
          }
      }
      
      if (GetOwner()->Implements<UMomentumInterface>())
      {
          return Cast<IMomentumInterface>(GetOwner());
      }
      
      return nullptr;
  }
  ```

## Updating an Existing Component

### Remove Direct References

- [ ] Identify direct references to other components:
  ```cpp
  // Before
  UPROPERTY()
  UMomentumComponent* MomentumComponent;
  ```

- [ ] Replace with interface helper methods:
  ```cpp
  // After
  IMomentumInterface* GetMomentumInterface() const;
  ```

- [ ] Update BeginPlay to remove component lookups:
  ```cpp
  // Before
  MomentumComponent = GetOwner()->FindComponentByClass<UMomentumComponent>();
  
  // After
  // No direct lookup needed
  ```

### Add Event System Integration

- [ ] Add event system reference and listener handles

- [ ] Initialize event system in BeginPlay

- [ ] Register for events in BeginPlay

- [ ] Clean up event listeners in EndPlay

- [ ] Add event handler methods

### Update Method Calls

- [ ] Replace direct method calls with interface calls:
  ```cpp
  // Before
  if (MomentumComponent)
  {
      MomentumComponent->AddMomentum(10.0f, FName("Action"));
  }
  
  // After
  IMomentumInterface* MomentumInterface = GetMomentumInterface();
  if (MomentumInterface)
  {
      IMomentumInterface::Execute_AddMomentum(MomentumInterface->_getUObject(), 10.0f, FName("Action"));
  }
  ```

- [ ] Replace direct state access with events:
  ```cpp
  // Before
  if (MomentumComponent && MomentumComponent->GetMomentumTier() >= 2)
  {
      // Do something
  }
  
  // After
  // Register for MomentumTierChanged events and handle in event handler
  ```

### Add Component Locator Integration

- [ ] Implement OnRegister and OnUnregister methods

- [ ] Register with component locator in OnRegister

- [ ] Unregister from component locator in OnUnregister

## Creating a New Interface

### Interface Definition

- [ ] Create interface header file:
  ```cpp
  #pragma once
  
  #include "CoreMinimal.h"
  #include "UObject/Interface.h"
  #include "YourInterface.generated.h"
  
  UINTERFACE(MinimalAPI, Blueprintable)
  class UYourInterface : public UInterface
  {
      GENERATED_BODY()
  };
  
  class IYourInterface
  {
      GENERATED_BODY()
  
  public:
      // Define interface methods
      UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Your Category")
      void YourMethod(float Param1, FName Param2);
  };
  ```

### Interface Implementation

- [ ] Add interface to component class declaration:
  ```cpp
  class RMC_API UYourComponent : public UActorComponent, public IYourInterface
  ```

- [ ] Add interface method implementations:
  ```cpp
  // In header
  virtual void YourMethod_Implementation(float Param1, FName Param2) override;
  
  // In cpp
  void UYourComponent::YourMethod_Implementation(float Param1, FName Param2)
  {
      // Implementation
  }
  ```

## Testing Your Implementation

### Basic Functionality Tests

- [ ] Test component creation and initialization

- [ ] Test event registration and handling

- [ ] Test interface method calls

- [ ] Test component locator registration and lookup

### Integration Tests

- [ ] Test interaction with other components

- [ ] Test event chaining between components

- [ ] Test complex scenarios involving multiple components

### Edge Cases

- [ ] Test behavior when dependencies are missing

- [ ] Test behavior during component lifecycle events (BeginPlay, EndPlay, etc.)

- [ ] Test behavior during game state changes (level loading, etc.)

## Code Review Checklist

### Architecture Compliance

- [ ] No direct references to other components

- [ ] Uses interfaces for component interaction

- [ ] Uses event system for state changes

- [ ] Registers with component locator

### Code Quality

- [ ] Proper error handling for missing dependencies

- [ ] Clean and consistent code style

- [ ] Appropriate comments and documentation

- [ ] No performance issues (e.g., expensive operations in tick)

### Lifecycle Management

- [ ] Proper initialization in BeginPlay

- [ ] Proper cleanup in EndPlay

- [ ] Proper registration in OnRegister

- [ ] Proper unregistration in OnUnregister

### Event Handling

- [ ] Event handlers have correct signatures

- [ ] Event handlers are marked with UFUNCTION macro

- [ ] Event listeners are properly cleaned up

### Interface Usage

- [ ] Interface methods have correct signatures

- [ ] Interface methods are properly implemented

- [ ] Interface methods are called correctly

## Conclusion

By following this checklist, you can ensure that your components properly integrate with the decoupled architecture. This will make your code more maintainable, extensible, and robust, while also making it easier for other developers to work with your components.

If you have any questions or need help implementing the architecture, please refer to the [Quick Start Guide](QUICK_START.md) or contact the architecture team.