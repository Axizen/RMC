// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WallRunInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UWallRunInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for wall run-related functionality.
 * 
 * This interface defines methods for accessing wall run information and performing wall run-related actions.
 * It allows components to interact with the WallRunComponent without direct references.
 */
class RMC_API IWallRunInterface
{
	GENERATED_BODY()

public:
	/**
	 * Checks if the character is currently wall running.
	 * 
	 * @return True if wall running, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	bool IsWallRunning() const;

	/**
	 * Gets the current wall run time.
	 * 
	 * @return The current wall run time
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetCurrentWallRunTime() const;

	/**
	 * Gets the maximum wall run duration.
	 * 
	 * @return The maximum wall run duration
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetMaxWallRunDuration() const;

	/**
	 * Checks if the character is wall running on the right side.
	 * 
	 * @return True if wall running on the right, false if on the left
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	bool IsWallRunningRight() const;

	/**
	 * Gets the current camera tilt due to wall running.
	 * 
	 * @return The current camera tilt
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetCurrentCameraTilt() const;

	/**
	 * Gets the wall run direction.
	 * 
	 * @return 1.0 for right wall, -1.0 for left wall, 0.0 for not wall running
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetWallRunDirection() const;

	/**
	 * Gets the wall run speed.
	 * 
	 * @return The wall run speed
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetWallRunSpeed() const;

	/**
	 * Gets the wall run gravity scale.
	 * 
	 * @return The wall run gravity scale
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetWallRunGravityScale() const;

	/**
	 * Gets the wall run momentum gain.
	 * 
	 * @return The wall run momentum gain
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetWallRunMomentumGain() const;

	/**
	 * Gets the wall jump off force.
	 * 
	 * @return The wall jump off force
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetWallJumpOffForce() const;

	/**
	 * Gets the wall jump up force.
	 * 
	 * @return The wall jump up force
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	float GetWallJumpUpForce() const;

	/**
	 * Checks if the character can wall run.
	 * 
	 * @return True if the character can wall run, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	bool CanWallRun() const;

	/**
	 * Tries to start wall running.
	 * 
	 * @return True if wall running started successfully, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	bool TryStartWallRun();

	/**
	 * Ends wall running.
	 * 
	 * @param bJumped Whether the wall run ended due to a jump
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	void EndWallRun(bool bJumped);

	/**
	 * Performs a wall jump.
	 * 
	 * @return True if the wall jump was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	bool PerformWallJump();

	/**
	 * Updates the camera tilt based on wall running.
	 * 
	 * @param DeltaTime The time since the last update
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "WallRun")
	void UpdateCameraTilt(float DeltaTime);
};