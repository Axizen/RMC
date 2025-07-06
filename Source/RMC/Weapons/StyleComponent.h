// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponEnums.h"
#include "../Core/StyleInterface.h"
#include "../Core/GameEventSystem.h"
#include "StyleComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UStyleComponent : public UActorComponent, public IStyleInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStyleComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Style properties
	UPROPERTY(BlueprintReadOnly, Category = "Style")
	float CurrentStylePoints;
	
	UPROPERTY(BlueprintReadOnly, Category = "Style")
	EStyleRank CurrentStyleRank;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float StyleDecayRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	TMap<EStyleRank, float> StyleRankThresholds;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	TMap<FName, float> MoveStyleValues;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	TMap<FName, float> MoveDiminishingReturns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float StylePointCap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float NoHitDecayMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float TimeSinceLastStyleGain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	float NoHitDecayDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	TMap<FName, float> LastMoveUseTimes;
	
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Style")
	void DecayStyle(float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category = "Style")
	void ResetStyle();
	
	UFUNCTION(BlueprintPure, Category = "Style")
	FText GetStyleRankText() const;
	
	// Implementation methods (called by interface methods)
	void AddStylePointsInternal(float Points, FName MoveName);
	float GetStyleMultiplierInternal() const;
	float GetStyleRankProgressInternal() const;
	float GetMoveStyleValueInternal(FName MoveName) const;

	UFUNCTION(BlueprintCallable, Category = "Style")
	void TakeDamage(float DamageAmount);
	
	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStyleRankChanged, EStyleRank, NewRank);
	UPROPERTY(BlueprintAssignable, Category = "Style|Events")
	FOnStyleRankChanged OnStyleRankChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStylePointsChanged, float, NewPoints, float, PointsAdded);
	UPROPERTY(BlueprintAssignable, Category = "Style|Events")
	FOnStylePointsChanged OnStylePointsChanged;

public:
	// Interface implementations
	// IStyleInterface
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float GetCurrentStylePoints() const;
	virtual float GetCurrentStylePoints_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	EStyleRank GetCurrentStyleRank() const;
	virtual EStyleRank GetCurrentStyleRank_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float GetStyleMultiplier() const;
	virtual float GetStyleMultiplier_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float GetStyleRankProgress() const;
	virtual float GetStyleRankProgress_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	float GetMoveStyleValue(FName MoveName) const;
	virtual float GetMoveStyleValue_Implementation(FName MoveName) const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void AddStylePoints(float Points, FName MoveName);
	virtual void AddStylePoints_Implementation(float Points, FName MoveName) override;

	// Component registration
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Event system methods
	class UGameEventSubsystem* GetEventSystem() const;
	void RegisterEventListeners();
	void UnregisterEventListeners();

	// Event handlers
	UFUNCTION()
	void OnPlayerDamaged(const FGameEventData& EventData);

	UFUNCTION()
	void OnEnemyDefeated(const FGameEventData& EventData);

	UFUNCTION()
	void OnWeaponFired(const FGameEventData& EventData);

	// Event broadcasting
	void BroadcastStylePointsGainedEvent(float Points, FName MoveName);
	void BroadcastStyleRankChangedEvent(EStyleRank NewRank);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Update style rank based on current points
	void UpdateStyleRank();

	// Calculate diminishing returns for repeated moves
	float CalculateDiminishingReturns(FName MoveName, float BasePoints);

	// Get the threshold for a specific rank
	float GetThresholdForRank(EStyleRank Rank) const;

	// Event system reference
	class UGameEventSubsystem* EventSystem;

	// Event listener handles
	TArray<int32> EventListenerHandles;
};