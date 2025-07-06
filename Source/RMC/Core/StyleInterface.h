// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StyleInterface.generated.h"

// Forward declarations
enum class EStyleRank : uint8;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UStyleInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for style-related functionality.
 * 
 * This interface defines methods for accessing style information and performing style-related actions.
 * It allows components to interact with the StyleComponent without direct references.
 */
class RMC_API IStyleInterface
{
	GENERATED_BODY()

public:
	/**
	 * Gets the current style points.
	 * 
	 * @return The current style points
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetCurrentStylePoints() const;

	/**
	 * Gets the current style rank.
	 * 
	 * @return The current style rank
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	uint8 GetCurrentStyleRank() const;

	/**
	 * Gets the style multiplier based on current rank.
	 * 
	 * @return The style multiplier
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetStyleMultiplier() const;

	/**
	 * Gets the progress towards the next style rank (0.0 to 1.0).
	 * 
	 * @return The style rank progress
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetStyleRankProgress() const;

	/**
	 * Gets the style rank as text.
	 * 
	 * @return The style rank text
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	FText GetStyleRankText() const;

	/**
	 * Gets the style value for a specific move.
	 * 
	 * @param MoveName The name of the move
	 * @return The style value for the move
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetMoveStyleValue(FName MoveName) const;

	/**
	 * Adds style points.
	 * 
	 * @param Points The amount of style points to add
	 * @param MoveName The name of the move that generated the points
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	void AddStylePoints(float Points, FName MoveName);

	/**
	 * Decays style over time.
	 * 
	 * @param DeltaTime The time since the last update
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	void DecayStyle(float DeltaTime);

	/**
	 * Resets style to default values.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	void ResetStyle();

	/**
	 * Handles the owner taking damage.
	 * 
	 * @param DamageAmount The amount of damage taken
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	void TakeDamage(float DamageAmount);

	/**
	 * Gets the style rank threshold for a specific rank.
	 * 
	 * @param Rank The style rank
	 * @return The threshold for the rank
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetThresholdForRank(uint8 Rank) const;

	/**
	 * Gets the style point cap.
	 * 
	 * @return The maximum style points
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetStylePointCap() const;

	/**
	 * Gets the style decay rate.
	 * 
	 * @return The style decay rate
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetStyleDecayRate() const;

	/**
	 * Gets the time since the last style gain.
	 * 
	 * @return The time since the last style gain
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Style")
	float GetTimeSinceLastStyleGain() const;
};