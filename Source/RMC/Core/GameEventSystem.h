// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameEventSystem.generated.h"

/**
 * Enum defining all possible event types in the game.
 * Add new event types here as needed.
 */
UENUM(BlueprintType)
enum class EGameEventType : uint8
{
	// Momentum events
	MomentumChanged UMETA(DisplayName = "Momentum Changed"),
	MomentumTierChanged UMETA(DisplayName = "Momentum Tier Changed"),
	OverchargeStateChanged UMETA(DisplayName = "Overcharge State Changed"),

	// Style events
	StylePointsGained UMETA(DisplayName = "Style Points Gained"),
	StyleRankChanged UMETA(DisplayName = "Style Rank Changed"),

	// Rift events
	RiftPerformed UMETA(DisplayName = "Rift Performed"),
	RiftStateChanged UMETA(DisplayName = "Rift State Changed"),

	// Wall Run events
	WallRunStarted UMETA(DisplayName = "Wall Run Started"),
	WallRunEnded UMETA(DisplayName = "Wall Run Ended"),
	WallJumpPerformed UMETA(DisplayName = "Wall Jump Performed"),

	// Weapon events
	WeaponFired UMETA(DisplayName = "Weapon Fired"),
	WeaponReloaded UMETA(DisplayName = "Weapon Reloaded"),
	WeaponSwitched UMETA(DisplayName = "Weapon Switched"),

	// Player events
	PlayerDamaged UMETA(DisplayName = "Player Damaged"),
	EnemyDefeated UMETA(DisplayName = "Enemy Defeated"),

	// Add more event types as needed
};

/**
 * Struct containing all data related to an event.
 * This is passed to event listeners when an event is broadcast.
 */
USTRUCT(BlueprintType)
struct FGameEventData
{
	GENERATED_BODY()

	// The type of event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	EGameEventType EventType;

	// The actor that instigated the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	AActor* Instigator = nullptr;

	// The target of the event (if applicable)
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	AActor* Target = nullptr;

	// Float value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	float FloatValue = 0.0f;

	// Int value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	int32 IntValue = 0;

	// Bool value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	bool BoolValue = false;

	// Name value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FName NameValue = NAME_None;

	// String value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString StringValue = FString();

	// Vector value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FVector VectorValue = FVector::ZeroVector;

	// Rotator value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FRotator RotatorValue = FRotator::ZeroRotator;

	// Transform value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FTransform TransformValue = FTransform::Identity;

	// Object value associated with the event
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	UObject* ObjectValue = nullptr;
};

// Delegate type for event listeners
DECLARE_DYNAMIC_DELEGATE_OneParam(FGameEventDelegate, const FGameEventData&, EventData);

/**
 * Subsystem that manages event broadcasting and listening between components.
 * This is the core of the decoupled architecture.
 */
UCLASS()
class RMC_API UGameEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	/**
	 * Broadcasts an event to all registered listeners.
	 * 
	 * @param EventData The data associated with the event
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void BroadcastEvent(const FGameEventData& EventData);

	/**
	 * Adds an event listener for a specific event type.
	 * 
	 * @param Listener The object that will receive the event
	 * @param EventType The type of event to listen for
	 * @param FunctionName The name of the function to call when the event occurs
	 * @return A handle that can be used to remove the listener
	 */
	FDelegateHandle AddEventListener(UObject* Listener, EGameEventType EventType, FName FunctionName);

	/**
	 * Removes an event listener.
	 * 
	 * @param Handle The handle returned by AddEventListener
	 */
	void RemoveEventListener(FDelegateHandle Handle);

	/**
	 * Removes all event listeners for a specific object.
	 * 
	 * @param Listener The object to remove listeners for
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void RemoveAllEventListeners(UObject* Listener);

	/**
	 * Adds a Blueprint event listener for a specific event type.
	 * 
	 * @param Listener The object that will receive the event
	 * @param EventType The type of event to listen for
	 * @param Callback The delegate to call when the event occurs
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void AddBlueprintEventListener(UObject* Listener, EGameEventType EventType, FGameEventDelegate Callback);

	/**
	 * Removes a Blueprint event listener.
	 * 
	 * @param Listener The object that registered the listener
	 * @param EventType The type of event the listener was registered for
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void RemoveBlueprintEventListener(UObject* Listener, EGameEventType EventType);

private:
	// Map of event types to arrays of delegates
	TMap<EGameEventType, TArray<TPair<FDelegateHandle, FScriptDelegate>>> EventListeners;

	// Map of event types to arrays of Blueprint delegates
	TMap<EGameEventType, TArray<TPair<UObject*, FGameEventDelegate>>> BlueprintEventListeners;

	// Map of delegate handles to event types for quick lookup
	TMap<FDelegateHandle, EGameEventType> HandleToEventType;

	// Helper function to create a script delegate
	FScriptDelegate CreateScriptDelegate(UObject* Listener, FName FunctionName);

	// Helper function to check if a function exists on an object
	bool DoesFunctionExist(UObject* Object, FName FunctionName);
};