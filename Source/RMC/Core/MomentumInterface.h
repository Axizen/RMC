// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MomentumInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UMomentumInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for momentum-related functionality.
 * 
 * This interface defines methods for accessing momentum information and performing momentum-related actions.
 * It allows components to interact with the MomentumComponent without direct references.
 */
class RMC_API IMomentumInterface
{
	GENERATED_BODY()

public:
	/**
	 * Gets the current momentum value.
	 * 
	 * @return The current momentum value
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float GetCurrentMomentum() const;

	/**
	 * Gets the current momentum tier.
	 * 
	 * @return The current momentum tier (0-based index)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	int32 GetMomentumTier() const;

	/**
	 * Gets the momentum ratio (0.0 to 1.0).
	 * 
	 * @return The momentum ratio
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float GetMomentumRatio() const;

	/**
	 * Gets the damage multiplier based on current momentum.
	 * 
	 * @return The damage multiplier
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float GetDamageMultiplier() const;

	/**
	 * Checks if the player is in overcharge state.
	 * 
	 * @return True if the player is overcharged, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	bool IsOvercharged() const;

	/**
	 * Adds momentum.
	 * 
	 * @param Amount The amount of momentum to add
	 * @param SourceName The name of the source of the momentum
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	void AddMomentum(float Amount, FName SourceName);

	/**
	 * Spends momentum.
	 * 
	 * @param Amount The amount of momentum to spend
	 * @return True if the momentum was spent, false if there wasn't enough
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	bool SpendMomentum(float Amount);

	/**
	 * Banks momentum for later use.
	 * 
	 * @param Amount The amount of momentum to bank
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	void BankMomentum(float Amount);

	/**
	 * Withdraws banked momentum.
	 * 
	 * @param Amount The amount of momentum to withdraw
	 * @return The actual amount withdrawn
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float WithdrawBankedMomentum(float Amount);

	/**
	 * Gets the amount of banked momentum.
	 * 
	 * @return The amount of banked momentum
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float GetBankedMomentum() const;

	/**
	 * Gets the maximum amount of momentum that can be banked.
	 * 
	 * @return The maximum amount of banked momentum
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float GetMaxBankedMomentum() const;

	/**
	 * Gets the maximum amount of momentum.
	 * 
	 * @return The maximum amount of momentum
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float GetMaxMomentum() const;

	/**
	 * Gets the momentum thresholds for each tier.
	 * 
	 * @return Array of momentum thresholds
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	TArray<float> GetMomentumThresholds() const;

	/**
	 * Checks if an action is allowed based on momentum.
	 * 
	 * @param ActionName The name of the action
	 * @param RequiredMomentum The amount of momentum required for the action
	 * @return True if the action is allowed, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	bool CanPerformMomentumAction(FName ActionName, float RequiredMomentum) const;
};