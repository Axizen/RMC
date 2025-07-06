// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RiftInterface.generated.h"

// Forward declarations
class ARiftAnchor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class URiftInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for rift-related functionality.
 * 
 * This interface defines methods for accessing rift information and performing rift-related actions.
 * It allows components to interact with the RiftComponent without direct references.
 */
class RMC_API IRiftInterface
{
	GENERATED_BODY()

public:
	/**
	 * Checks if the component is currently in cooldown.
	 * 
	 * @return True if in cooldown, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool IsInCooldown() const;

	/**
	 * Checks if the component is currently rifting.
	 * 
	 * @return True if rifting, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool IsRifting() const;

	/**
	 * Gets the current rift state.
	 * 
	 * @return The current rift state
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	uint8 GetRiftState() const;

	/**
	 * Gets the current chain count.
	 * 
	 * @return The current chain count
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	int32 GetCurrentChainCount() const;

	/**
	 * Gets the maximum rift distance.
	 * 
	 * @return The maximum rift distance
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	float GetMaxRiftDistance() const;

	/**
	 * Gets the maximum chain count.
	 * 
	 * @return The maximum chain count
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	int32 GetMaxChainCount() const;

	/**
	 * Gets the phantom dodge duration.
	 * 
	 * @return The phantom dodge duration
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	float GetPhantomDodgeDuration() const;

	/**
	 * Gets the phantom dodge distance.
	 * 
	 * @return The phantom dodge distance
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	float GetPhantomDodgeDistance() const;

	/**
	 * Checks if aerial reset is available.
	 * 
	 * @return True if aerial reset is available, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool CanAerialReset() const;

	/**
	 * Checks if counter rift is available.
	 * 
	 * @return True if counter rift is available, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool CanCounterRift() const;

	/**
	 * Performs a phantom dodge in the specified direction.
	 * 
	 * @param Direction The direction to dodge in
	 * @param bIsAerial Whether the dodge is performed in the air
	 * @return True if the dodge was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool PerformPhantomDodge(FVector Direction, bool bIsAerial);

	/**
	 * Initiates a rift tether to a target location.
	 * 
	 * @param TargetLocation The location to tether to
	 * @return True if the tether was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool InitiateRiftTether(const FVector& TargetLocation);

	/**
	 * Initiates a rift tether to a specific anchor.
	 * 
	 * @param TargetAnchor The anchor to tether to
	 * @return True if the tether was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool InitiateRiftTetherToAnchor(ARiftAnchor* TargetAnchor);

	/**
	 * Attempts to chain to the next available rift anchor.
	 * 
	 * @return True if the chain was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool ChainRiftTether();

	/**
	 * Performs a counter rift if timing is correct.
	 * 
	 * @return True if the counter rift was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool AttemptCounterRift();

	/**
	 * Notifies the component that the owner has taken damage.
	 * 
	 * @param DamageAmount The amount of damage taken
	 * @param DamageCauser The actor that caused the damage
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	void NotifyTakeDamage(float DamageAmount, AActor* DamageCauser);

	/**
	 * Finds the best rift anchor in range.
	 * 
	 * @return The best rift anchor, or nullptr if none found
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	ARiftAnchor* FindBestRiftAnchor() const;
};