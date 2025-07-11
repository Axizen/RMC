// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RMCMomentumBased.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class RMC_API URMCMomentumBased : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for momentum-based movement
 */
class RMC_API IRMCMomentumBased
{
	GENERATED_BODY()

public:
	// Get current momentum value
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
	float GetCurrentMomentum() const;
	
	// Add momentum
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
	void AddMomentum(float Amount);
	
	// Reduce momentum
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
	void ReduceMomentum(float Amount);
	
	// Check if has minimum momentum for an action
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
	bool HasMinimumMomentumForAction(float RequiredMomentum) const;
	
	// Get momentum as a percentage of maximum
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
	float GetMomentumPercent() const;
};