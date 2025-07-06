// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RailGrindInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class URailGrindInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for rail grinding functionality.
 * 
 * This interface defines methods for accessing rail grinding information and performing rail grinding actions.
 * It allows components to interact with the RailGrindComponent without direct references.
 */
class RMC_API IRailGrindInterface
{
	GENERATED_BODY()

public:
	/**
	 * Checks if the character is currently grinding on a rail.
	 * 
	 * @return True if grinding, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool IsRailGrinding() const;

	/**
	 * Gets the current rail grinding speed.
	 * 
	 * @return The current rail grinding speed
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetRailGrindSpeed() const;

	/**
	 * Gets the base rail grinding speed (before momentum modifiers).
	 * 
	 * @return The base rail grinding speed
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetBaseRailGrindSpeed() const;

	/**
	 * Gets the current rail grinding time.
	 * 
	 * @return The current rail grinding time
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetCurrentRailGrindTime() const;

	/**
	 * Checks if the character can start grinding on a rail.
	 * 
	 * @return True if the character can start grinding, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool CanStartRailGrind() const;

	/**
	 * Tries to start rail grinding.
	 * 
	 * @return True if rail grinding started successfully, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool TryStartRailGrind();

	/**
	 * Ends rail grinding.
	 * 
	 * @param bJumped Whether the rail grinding ended due to a jump
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	void EndRailGrind(bool bJumped);

	/**
	 * Performs a rail jump.
	 * 
	 * @return True if the rail jump was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool PerformRailJump();

	/**
	 * Performs a rail trick.
	 * 
	 * @param TrickName The name of the trick to perform
	 * @return True if the trick was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool PerformRailTrick(FName TrickName);

	/**
	 * Switches to another rail.
	 * 
	 * @param Direction The direction to switch (1 for right, -1 for left)
	 * @return True if the rail switch was successful, false otherwise
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool SwitchRail(int32 Direction);

	/**
	 * Gets the current rail actor.
	 * 
	 * @return The current rail actor, or nullptr if not grinding
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	class ARailSplineActor* GetCurrentRail() const;

	/**
	 * Gets the current position along the rail (0.0 to 1.0).
	 * 
	 * @return The current position along the rail
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetRailPosition() const;

	/**
	 * Gets the current rail direction.
	 * 
	 * @return The current rail direction vector
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	FVector GetRailDirection() const;
};