// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameEventSystem.h"
#include "MomentumEffectsComponent.generated.h"

class UNiagaraSystem;
class USoundBase;

/**
 * MomentumEffectsComponent
 * 
 * This component demonstrates how to use the decoupled architecture to respond to momentum changes
 * without direct references to the MomentumComponent. It plays visual and audio effects based on
 * momentum changes, momentum tier changes, and overcharge state changes.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMomentumEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMomentumEffectsComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called when the component is registered
	virtual void OnRegister() override;

	// Called when the component is unregistered
	virtual void OnUnregister() override;

	// Called when the component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Momentum tier effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	TMap<int32, UNiagaraSystem*> MomentumTierEffects;

	// Momentum tier sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	TMap<int32, USoundBase*> MomentumTierSounds;

	// Overcharge effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	UNiagaraSystem* OverchargeEffect;

	// Overcharge sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	USoundBase* OverchargeSound;

	// Momentum gain effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	UNiagaraSystem* MomentumGainEffect;

	// Momentum gain sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	USoundBase* MomentumGainSound;

	// Momentum loss effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	UNiagaraSystem* MomentumLossEffect;

	// Momentum loss sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	USoundBase* MomentumLossSound;

	// Minimum momentum change to trigger effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Momentum")
	float MinMomentumChangeForEffect;

	// Play a momentum tier effect
	UFUNCTION(BlueprintCallable, Category = "Effects|Momentum")
	void PlayMomentumTierEffect(int32 Tier);

	// Play an overcharge effect
	UFUNCTION(BlueprintCallable, Category = "Effects|Momentum")
	void PlayOverchargeEffect(bool bIsOvercharged);

	// Play a momentum change effect
	UFUNCTION(BlueprintCallable, Category = "Effects|Momentum")
	void PlayMomentumChangeEffect(float NewMomentum, float MomentumDelta);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Event system reference
	UPROPERTY()
	UGameEventSubsystem* EventSystem;

	// Event listener handles
	TArray<FEventListenerHandle> EventListenerHandles;

	// Event handlers
	UFUNCTION()
	void OnMomentumChanged(const FGameEventData& EventData);

	UFUNCTION()
	void OnMomentumTierChanged(const FGameEventData& EventData);

	UFUNCTION()
	void OnOverchargeStateChanged(const FGameEventData& EventData);

	// Helper function to spawn effects
	void SpawnEffect(UNiagaraSystem* Effect, USoundBase* Sound);
};