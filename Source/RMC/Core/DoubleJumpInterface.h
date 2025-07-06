// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DoubleJumpInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UDoubleJumpInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for double jump functionality.
 * 
 * This interface defines methods for accessing double jump information and performing double jump actions.
 * It allows components to interact with the DoubleJumpComponent without direct references.
 */
class RMC_API IDoubleJumpInterface
{
	GENERATED_BODY()

public:
	/**
	 * Checks if the character can perform a double jump.
	 * 
	 * @return True if double jump is available, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	bool CanDoubleJump() const;

	/**
	 * Performs a double jump.
	 * 
	 * @return True if the double jump was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	bool PerformDoubleJump();

	/**
	 * Gets the current double jump count.
	 * 
	 * @return The number of double jumps performed since last touching the ground
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	int32 GetDoubleJumpCount() const;

	/**
	 * Gets the maximum number of double jumps allowed.
	 * 
	 * @return The maximum number of double jumps
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	int32 GetMaxDoubleJumps() const;

	/**
	 * Gets the double jump force.
	 * 
	 * @return The force applied when double jumping
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	float GetDoubleJumpForce() const;

	/**
	 * Resets the double jump count.
	 * Called when the character lands on the ground.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	void ResetDoubleJumpCount();

	/**
	 * Gets whether the character is currently in a double jump.
	 * 
	 * @return True if the character is in a double jump, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	bool IsDoubleJumping() const;
};