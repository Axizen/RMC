// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/MomentumInterface.h"
#include "Core/GameEventSystem.h"
#include "MomentumComponent.generated.h"

// Platforming momentum source constants
#define MOMENTUM_SOURCE_WALL_RUN FName("WallRun")
#define MOMENTUM_SOURCE_DOUBLE_JUMP FName("DoubleJump")
#define MOMENTUM_SOURCE_WALL_BOUNCE FName("WallBounce")
#define MOMENTUM_SOURCE_RAIL_GRIND FName("RailGrind")
#define MOMENTUM_SOURCE_RIFT_CHAIN FName("RiftChain")

// Struct for platforming ability thresholds
USTRUCT(BlueprintType)
struct FPlatformingAbilityThreshold
{
	GENERATED_BODY()

	// Name of the ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platforming")
	FName AbilityName;

	// Required momentum to use this ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platforming")
	float RequiredMomentum;

	// Cooldown time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platforming")
	float Cooldown;

	// Whether this ability requires a specific momentum tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platforming")
	bool bRequiresTier;

	// Minimum momentum tier required (if bRequiresTier is true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platforming")
	int32 RequiredTier;

	// Constructor
	FPlatformingAbilityThreshold()
		: AbilityName(NAME_None)
		, RequiredMomentum(0.0f)
		, Cooldown(0.0f)
		, bRequiresTier(false)
		, RequiredTier(0)
	{
	}

	// Constructor with parameters
	FPlatformingAbilityThreshold(FName InName, float InRequiredMomentum, float InCooldown, bool InRequiresTier = false, int32 InRequiredTier = 0)
		: AbilityName(InName)
		, RequiredMomentum(InRequiredMomentum)
		, Cooldown(InCooldown)
		, bRequiresTier(InRequiresTier)
		, RequiredTier(InRequiredTier)
	{
	}
};

class UGameEventSubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMomentumComponent : public UActorComponent, public IMomentumInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMomentumComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Current momentum value
	UPROPERTY(BlueprintReadOnly, Category = "Momentum")
	float CurrentMomentum;
	
	// Maximum momentum value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	float MaxMomentum;
	
	// Momentum decay rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	float MomentumDecayRate;
	
	// Momentum gain multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	float MomentumGainMultiplier;
	
	// Momentum threshold tiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	TArray<float> MomentumThresholds;

	// Banked momentum (stored for later use)
	UPROPERTY(BlueprintReadOnly, Category = "Momentum")
	float BankedMomentum;

	// Maximum banked momentum
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	float MaxBankedMomentum;

	// Whether the player is in overcharge state
	UPROPERTY(BlueprintReadOnly, Category = "Momentum")
	bool bIsOvercharged;

	// Threshold for overcharge state (percentage of max momentum)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	float OverchargeThreshold;

	// Damage multiplier during overcharge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	float OverchargeDamageMultiplier;

	// Damage taken multiplier during overcharge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
	float OverchargeDamageTakenMultiplier;

	// Platforming-specific momentum gain values
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum|Platforming")
	float WallRunMomentumGain = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum|Platforming")
	float DoubleJumpMomentumGain = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum|Platforming")
	float WallBounceMomentumGain = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum|Platforming")
	float RailGrindMomentumGain = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum|Platforming")
	float RiftChainMomentumGain = 12.0f;

	// Platforming ability thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum|Platforming")
	TArray<FPlatformingAbilityThreshold> PlatformingAbilities;

	// Whether momentum preservation is active
	UPROPERTY(BlueprintReadOnly, Category = "Momentum|Platforming")
	bool bMomentumPreservationActive = false;

	// Maximum time momentum preservation can be active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum|Platforming")
	float MaxMomentumPreservationTime = 2.0f;

	// Functions to add/remove momentum
	UFUNCTION(BlueprintCallable, Category = "Momentum")
	void DecayMomentum(float DeltaTime);

	// Momentum spending and banking
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	bool SpendMomentum(float Amount);
	virtual bool SpendMomentum_Implementation(float Amount) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	void BankMomentum(float Amount);
	virtual void BankMomentum_Implementation(float Amount) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Momentum")
	float WithdrawBankedMomentum(float Amount);
	virtual float WithdrawBankedMomentum_Implementation(float Amount) override;

	// Handle taking damage
	UFUNCTION(BlueprintCallable, Category = "Momentum")
	void OnTakeDamage(float DamageAmount);

	// Platforming-specific momentum functions
	
	/**
	 * Adds momentum from a wall run action
	 * @param Duration - How long the wall run has been active
	 * @return The amount of momentum added
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	float AddWallRunMomentum(float Duration);

	/**
	 * Adds momentum from a double jump action
	 * @return The amount of momentum added
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	float AddDoubleJumpMomentum();

	/**
	 * Adds momentum from a wall bounce action
	 * @return The amount of momentum added
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	float AddWallBounceMomentum();

	/**
	 * Adds momentum from a rail grind action
	 * @param Duration - How long the rail grind has been active
	 * @return The amount of momentum added
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	float AddRailGrindMomentum(float Duration);

	/**
	 * Adds momentum from a rift chain action
	 * @param ChainCount - The current chain count
	 * @return The amount of momentum added
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	float AddRiftChainMomentum(int32 ChainCount);

	/**
	 * Temporarily pauses momentum decay during platforming actions
	 * @param Duration - How long to preserve momentum (capped by MaxMomentumPreservationTime)
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	void PreserveMomentum(float Duration);

	/**
	 * Resumes momentum decay
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	void ResumeMomentumDecay();

	/**
	 * Checks if a specific platforming ability is available based on current momentum
	 * @param AbilityName - The name of the ability to check
	 * @return True if the ability is available
	 */
	UFUNCTION(BlueprintPure, Category = "Momentum|Platforming")
	bool IsPlatformingAbilityAvailable(FName AbilityName) const;

	/**
	 * Gets the current momentum color for visual effects
	 * @return A color based on the current momentum level
	 */
	UFUNCTION(BlueprintPure, Category = "Momentum|Platforming")
	FLinearColor GetMomentumColor() const;

	/**
	 * Gets the momentum intensity for visual effects (0.0 to 1.0)
	 * @return Intensity value based on current momentum
	 */
	UFUNCTION(BlueprintPure, Category = "Momentum|Platforming")
	float GetMomentumIntensity() const;

	/**
	 * Triggers a momentum pulse effect for significant momentum gains
	 * @param Intensity - The intensity of the pulse (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Momentum|Platforming")
	void TriggerMomentumPulse(float Intensity = 1.0f);
	
	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMomentumTierChanged, int32, NewTier);
	UPROPERTY(BlueprintAssignable, Category = "Momentum|Events")
	FOnMomentumTierChanged OnMomentumTierChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMomentumChanged, float, NewMomentum, float, MomentumDelta);
	UPROPERTY(BlueprintAssignable, Category = "Momentum|Events")
	FOnMomentumChanged OnMomentumChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverchargeStateChanged, bool, bIsOvercharged);
	UPROPERTY(BlueprintAssignable, Category = "Momentum|Events")
	FOnOverchargeStateChanged OnOverchargeStateChanged;

	// Platforming-specific events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMomentumPreservationChanged, bool, bIsActive);
	UPROPERTY(BlueprintAssignable, Category = "Momentum|Platforming")
	FOnMomentumPreservationChanged OnMomentumPreservationChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlatformingAbilityAvailabilityChanged, FName, AbilityName, bool, bIsAvailable);
	UPROPERTY(BlueprintAssignable, Category = "Momentum|Platforming")
	FOnPlatformingAbilityAvailabilityChanged OnPlatformingAbilityAvailabilityChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMomentumPulseTriggered, float, Intensity);
	UPROPERTY(BlueprintAssignable, Category = "Momentum|Platforming")
	FOnMomentumPulseTriggered OnMomentumPulseTriggered;

	// IMomentumInterface implementation
	virtual float GetCurrentMomentum_Implementation() const override { return CurrentMomentum; }
	virtual int32 GetMomentumTier_Implementation() const override;
	virtual float GetMomentumRatio_Implementation() const override;
	virtual float GetDamageMultiplier_Implementation() const override;
	virtual bool CanPerformMomentumAction_Implementation(FName ActionName, float RequiredMomentum) const override;
	virtual void AddMomentum_Implementation(float Amount, FName SourceName) override;
	virtual bool IsOvercharged_Implementation() const override { return bIsOvercharged; }
	virtual float GetBankedMomentum_Implementation() const override { return BankedMomentum; }
	virtual float GetMaxBankedMomentum_Implementation() const override { return MaxBankedMomentum; }
	virtual float GetMaxMomentum_Implementation() const override { return MaxMomentum; }
	virtual TArray<float> GetMomentumThresholds_Implementation() const override { return MomentumThresholds; }

	// Event handlers for game events
	UFUNCTION()
	void OnStylePointsGained(const FGameEventData& EventData);
	
	UFUNCTION()
	void OnPlayerDamaged(const FGameEventData& EventData);
	
	UFUNCTION()
	void OnEnemyDefeated(const FGameEventData& EventData);
	
	UFUNCTION()
	void OnWeaponFired(const FGameEventData& EventData);

	// Register with the component locator
	virtual void OnRegister() override;
	
	// Unregister from the component locator
	virtual void OnUnregister() override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called when the component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Previous momentum tier for change detection
	int32 PreviousMomentumTier;

	// Previous overcharge state for change detection
	bool bWasOvercharged;

	// Update overcharge state
	void UpdateOverchargeState();

	// Last time momentum was gained
	float LastMomentumGainTime;

	// Momentum sources and their last use times
	TMap<FName, float> MomentumSourceLastUseTimes;

	// Diminishing returns for repeated momentum sources
	UFUNCTION()
	float CalculateDiminishingReturns(FName SourceName, float BaseAmount);
	
	// Reference to the game event subsystem
	UPROPERTY()
	UGameEventSubsystem* EventSystem;
	
	// Register for events
	void RegisterForEvents();
	
	// Unregister from events
	void UnregisterFromEvents();
	
	// Array of event listener handles
	TArray<int32> EventListenerHandles;
	
	// Broadcast momentum changed event through the event system
	void BroadcastMomentumChanged(float NewMomentum, float MomentumDelta);
	
	// Broadcast momentum tier changed event through the event system
	void BroadcastMomentumTierChanged(int32 NewTier);
	
	// Broadcast overcharge state changed event through the event system
	void BroadcastOverchargeStateChanged(bool bNewOverchargeState);

	// Platforming-specific protected members

	// Timer handle for momentum preservation
	FTimerHandle MomentumPreservationTimerHandle;

	// Time when momentum preservation started
	float MomentumPreservationStartTime;

	// Current momentum preservation duration
	float CurrentMomentumPreservationDuration;

	// Last time a momentum pulse was triggered
	float LastMomentumPulseTime;

	// Minimum time between momentum pulses
	float MomentumPulseCooldown;

	// Map of platforming ability cooldowns
	TMap<FName, float> PlatformingAbilityCooldowns;

	// Map of previously available platforming abilities for change detection
	TMap<FName, bool> PreviousAbilityAvailability;

	// Callback for when momentum preservation timer expires
	UFUNCTION()
	void OnMomentumPreservationTimerExpired();

	// Update platforming ability availability
	void UpdatePlatformingAbilityAvailability();

	// Broadcast platforming ability availability changed event
	void BroadcastPlatformingAbilityAvailabilityChanged(FName AbilityName, bool bIsAvailable);

	// Broadcast momentum preservation changed event
	void BroadcastMomentumPreservationChanged(bool bIsActive);

	// Broadcast momentum pulse triggered event
	void BroadcastMomentumPulseTriggered(float Intensity);
};