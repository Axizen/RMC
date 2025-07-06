// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameEventSystem.h"
#include "MomentumEffectsExample.generated.h"

/**
 * Example component that provides methods to simulate momentum changes for testing purposes.
 * 
 * This component demonstrates how to broadcast events in the decoupled architecture.
 * It can be used to test momentum-related effects without needing an actual MomentumComponent.
 * 
 * Broadcasts events:
 * - MomentumChanged: When SimulateMomentumChange is called
 * - MomentumTierChanged: When SimulateMomentumTierChange is called
 * - OverchargeStateChanged: When SimulateOverchargeStateChange is called
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMomentumEffectsExample : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMomentumEffectsExample();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Simulates a momentum change event.
	 * 
	 * @param MomentumValue The new momentum value
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Testing")
	void SimulateMomentumChange(float MomentumValue);

	/**
	 * Simulates a momentum tier change event.
	 * 
	 * @param NewTier The new momentum tier
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Testing")
	void SimulateMomentumTierChange(int32 NewTier);

	/**
	 * Simulates an overcharge state change event.
	 * 
	 * @param bIsOvercharged Whether the player is overcharged
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Testing")
	void SimulateOverchargeStateChange(bool bIsOvercharged);

	/**
	 * Simulates a momentum oscillation over time.
	 * 
	 * @param MinValue The minimum momentum value
	 * @param MaxValue The maximum momentum value
	 * @param Period The oscillation period in seconds
	 * @param bAutoStart Whether to start the oscillation automatically
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Testing")
	void SimulateMomentumOscillation(float MinValue = 0.0f, float MaxValue = 100.0f, float Period = 5.0f, bool bAutoStart = true);

	/**
	 * Starts the momentum oscillation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Testing")
	void StartMomentumOscillation();

	/**
	 * Stops the momentum oscillation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Testing")
	void StopMomentumOscillation();

	/**
	 * Simulates a complete momentum cycle with tier changes and overcharge.
	 * 
	 * @param CycleDuration The duration of the complete cycle in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Testing")
	void SimulateCompleteMomentumCycle(float CycleDuration = 10.0f);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called when the component is being destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Event system reference
	UGameEventSubsystem* EventSystem;

	// Oscillation parameters
	float OscillationMinValue;
	float OscillationMaxValue;
	float OscillationPeriod;
	bool bOscillationActive;
	float OscillationTime;

	// Complete cycle parameters
	bool bCycleActive;
	float CycleTime;
	float CycleDuration;

	// Timer handles
	FTimerHandle OscillationTimerHandle;
	FTimerHandle CycleTimerHandle;

	// Helper functions
	void UpdateOscillation(float DeltaTime);
	void UpdateCycle(float DeltaTime);
	void BroadcastMomentumChangedEvent(float MomentumValue);
	void BroadcastMomentumTierChangedEvent(int32 NewTier);
	void BroadcastOverchargeStateChangedEvent(bool bIsOvercharged);
};