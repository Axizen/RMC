# Platcom Framework Event System Reference

## Overview

This document provides a comprehensive reference for all event types in the Platcom Framework's event system. It includes details on each event type, the data it carries, and how to use it effectively.

## Event Types

### Momentum Events

#### `EGameEventType::MomentumChanged`

**Description**: Broadcast when a character's momentum value changes.

**Data**:
- `FloatValue`: The new momentum value
- `BoolValue`: True if momentum increased, false if it decreased
- `Instigator`: The actor whose momentum changed

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::MomentumChanged;
EventData.Instigator = GetOwner();
EventData.FloatValue = CurrentMomentum;
EventData.BoolValue = (MomentumDelta > 0.0f);
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::MomentumChanged, this, FName("OnMomentumChanged"));

// Handler
void UMyComponent::OnMomentumChanged(const FGameEventData& EventData)
{
    float NewMomentum = EventData.FloatValue;
    bool bGained = EventData.BoolValue;
    // Handle momentum change
}
```

#### `EGameEventType::MomentumTierChanged`

**Description**: Broadcast when a character's momentum tier changes.

**Data**:
- `IntValue`: The new momentum tier
- `Instigator`: The actor whose momentum tier changed

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::MomentumTierChanged;
EventData.Instigator = GetOwner();
EventData.IntValue = NewTier;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::MomentumTierChanged, this, FName("OnMomentumTierChanged"));

// Handler
void UMyComponent::OnMomentumTierChanged(const FGameEventData& EventData)
{
    int32 NewTier = EventData.IntValue;
    // Handle tier change
}
```

### Style Events

#### `EGameEventType::StylePointsGained`

**Description**: Broadcast when a character gains style points.

**Data**:
- `FloatValue`: The amount of style points gained
- `NameValue`: The name of the move that generated the points
- `Instigator`: The actor that gained style points

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::StylePointsGained;
EventData.Instigator = GetOwner();
EventData.FloatValue = Points;
EventData.NameValue = MoveName;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::StylePointsGained, this, FName("OnStylePointsGained"));

// Handler
void UMyComponent::OnStylePointsGained(const FGameEventData& EventData)
{
    float Points = EventData.FloatValue;
    FName MoveName = EventData.NameValue;
    // Handle style points gain
}
```

#### `EGameEventType::StyleRankChanged`

**Description**: Broadcast when a character's style rank changes.

**Data**:
- `IntValue`: The new style rank (as an integer representation of EStyleRank)
- `Instigator`: The actor whose style rank changed

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::StyleRankChanged;
EventData.Instigator = GetOwner();
EventData.IntValue = static_cast<int32>(NewRank);
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::StyleRankChanged, this, FName("OnStyleRankChanged"));

// Handler
void UMyComponent::OnStyleRankChanged(const FGameEventData& EventData)
{
    EStyleRank NewRank = static_cast<EStyleRank>(EventData.IntValue);
    // Handle rank change
}
```

### Weapon Events

#### `EGameEventType::WeaponFired`

**Description**: Broadcast when a weapon is fired.

**Data**:
- `Instigator`: The actor that fired the weapon
- `NameValue`: The name of the weapon or fire mode

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::WeaponFired;
EventData.Instigator = GetOwner();
EventData.NameValue = WeaponName;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::WeaponFired, this, FName("OnWeaponFired"));

// Handler
void UMyComponent::OnWeaponFired(const FGameEventData& EventData)
{
    FName WeaponName = EventData.NameValue;
    // Handle weapon fired
}
```

#### `EGameEventType::WeaponReloaded`

**Description**: Broadcast when a weapon is reloaded.

**Data**:
- `Instigator`: The actor that reloaded the weapon
- `NameValue`: The name of the weapon

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::WeaponReloaded;
EventData.Instigator = GetOwner();
EventData.NameValue = WeaponName;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::WeaponReloaded, this, FName("OnWeaponReloaded"));

// Handler
void UMyComponent::OnWeaponReloaded(const FGameEventData& EventData)
{
    FName WeaponName = EventData.NameValue;
    // Handle weapon reloaded
}
```

### Rift Events

#### `EGameEventType::RiftPerformed`

**Description**: Broadcast when a rift ability is performed.

**Data**:
- `Instigator`: The actor that performed the rift
- `LocationValue`: The target location of the rift
- `NameValue`: The type of rift ("Tether", "Dodge", etc.)

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::RiftPerformed;
EventData.Instigator = GetOwner();
EventData.LocationValue = TargetLocation;
EventData.NameValue = FName("Tether");
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::RiftPerformed, this, FName("OnRiftPerformed"));

// Handler
void UMyComponent::OnRiftPerformed(const FGameEventData& EventData)
{
    FVector TargetLocation = EventData.LocationValue;
    FName RiftType = EventData.NameValue;
    // Handle rift performed
}
```

#### `EGameEventType::RiftStateChanged`

**Description**: Broadcast when a character's rift state changes.

**Data**:
- `Instigator`: The actor whose rift state changed
- `IntValue`: The new rift state (as an integer representation of ERiftState)

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::RiftStateChanged;
EventData.Instigator = GetOwner();
EventData.IntValue = static_cast<int32>(NewState);
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::RiftStateChanged, this, FName("OnRiftStateChanged"));

// Handler
void UMyComponent::OnRiftStateChanged(const FGameEventData& EventData)
{
    ERiftState NewState = static_cast<ERiftState>(EventData.IntValue);
    // Handle rift state change
}
```

### Wall Run Events

#### `EGameEventType::WallRunStarted`

**Description**: Broadcast when a character starts wall running.

**Data**:
- `Instigator`: The actor that started wall running
- `BoolValue`: True if running on right wall, false if running on left wall

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::WallRunStarted;
EventData.Instigator = GetOwner();
EventData.BoolValue = bIsWallRunningRight;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::WallRunStarted, this, FName("OnWallRunStarted"));

// Handler
void UMyComponent::OnWallRunStarted(const FGameEventData& EventData)
{
    bool bIsWallRunningRight = EventData.BoolValue;
    // Handle wall run started
}
```

#### `EGameEventType::WallRunEnded`

**Description**: Broadcast when a character stops wall running.

**Data**:
- `Instigator`: The actor that stopped wall running
- `BoolValue`: True if ended with a jump, false otherwise

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::WallRunEnded;
EventData.Instigator = GetOwner();
EventData.BoolValue = bJumped;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::WallRunEnded, this, FName("OnWallRunEnded"));

// Handler
void UMyComponent::OnWallRunEnded(const FGameEventData& EventData)
{
    bool bJumped = EventData.BoolValue;
    // Handle wall run ended
}
```

### Damage Events

#### `EGameEventType::PlayerDamaged`

**Description**: Broadcast when the player takes damage.

**Data**:
- `Instigator`: The actor that took damage
- `Target`: The actor that caused the damage
- `FloatValue`: The amount of damage taken

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::PlayerDamaged;
EventData.Instigator = this;
EventData.Target = DamageCauser;
EventData.FloatValue = DamageAmount;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::PlayerDamaged, this, FName("OnPlayerDamaged"));

// Handler
void UMyComponent::OnPlayerDamaged(const FGameEventData& EventData)
{
    float DamageAmount = EventData.FloatValue;
    AActor* DamageCauser = EventData.Target;
    // Handle player damaged
}
```

#### `EGameEventType::EnemyDamaged`

**Description**: Broadcast when an enemy takes damage.

**Data**:
- `Instigator`: The actor that caused the damage
- `Target`: The actor that took damage
- `FloatValue`: The amount of damage dealt

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::EnemyDamaged;
EventData.Instigator = GetInstigator();
EventData.Target = DamagedActor;
EventData.FloatValue = DamageAmount;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::EnemyDamaged, this, FName("OnEnemyDamaged"));

// Handler
void UMyComponent::OnEnemyDamaged(const FGameEventData& EventData)
{
    float DamageAmount = EventData.FloatValue;
    AActor* DamagedActor = EventData.Target;
    // Handle enemy damaged
}
```

#### `EGameEventType::EnemyDefeated`

**Description**: Broadcast when an enemy is defeated.

**Data**:
- `Instigator`: The actor that defeated the enemy
- `Target`: The enemy that was defeated

**Usage Example**:
```cpp
// Broadcasting
FGameEventData EventData;
EventData.EventType = EGameEventType::EnemyDefeated;
EventData.Instigator = GetInstigator();
EventData.Target = DefeatedEnemy;
EventSystem->BroadcastEvent(EventData);

// Listening
EventSystem->AddEventListenerWithObject(EGameEventType::EnemyDefeated, this, FName("OnEnemyDefeated"));

// Handler
void UMyComponent::OnEnemyDefeated(const FGameEventData& EventData)
{
    AActor* DefeatedEnemy = EventData.Target;
    // Handle enemy defeated
}
```

## Best Practices

### Event Data Usage

1. **Be Consistent**: Use the same data fields for the same purpose across all events.
2. **Include All Relevant Data**: Make sure events include all the data that listeners might need.
3. **Use Appropriate Fields**: Use the field that best matches the data type (e.g., use `FloatValue` for floating-point values).
4. **Document Events**: Keep this reference up to date as new events are added.

### Event Broadcasting

1. **Check for Event System**: Always check if the event system is valid before broadcasting.
2. **Broadcast at Appropriate Times**: Don't broadcast events too frequently, as it can impact performance.
3. **Use Descriptive Event Types**: Create specific event types for different kinds of events.

### Event Listening

1. **Register in BeginPlay**: Register for events in `BeginPlay` or similar initialization functions.
2. **Unregister in EndPlay**: Always unregister from events in `EndPlay` to prevent memory leaks.
3. **Store Listener Handles**: Store the handles returned by `AddEventListener` for later cleanup.
4. **Use Appropriate Handler Signature**: Make sure event handlers have the correct signature.

## Extending the Event System

To add new event types:

1. Add the new event type to the `EGameEventType` enum in `GameEventSystem.h`.
2. Document the new event type in this reference.
3. Use the new event type in your code.

Example:
```cpp
// In GameEventSystem.h
enum class EGameEventType : uint8
{
    // Existing event types...
    MyNewEvent UMETA(DisplayName = "My New Event"),
};
```

## Conclusion

The event system provides a powerful way to decouple components in the Platcom Framework. By using events instead of direct references, components can communicate without knowing about each other, making the codebase more maintainable and extensible.

This reference should be kept up to date as new event types are added to the system.