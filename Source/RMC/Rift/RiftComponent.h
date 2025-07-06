// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RiftTypes.h"
#include "../Core/RiftInterface.h"
#include "../Core/MomentumInterface.h"
#include "../Core/StyleInterface.h"
#include "RiftComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API URiftComponent : public UActorComponent, public IRiftInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URiftComponent();

	// Called when the component is registered
	virtual void OnRegister() override;

	// Called when the component is unregistered
	virtual void OnUnregister() override;

	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// IRiftInterface implementation
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool CanRift() const;
	virtual bool CanRift_Implementation() const override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	ERiftState GetRiftState() const;
	virtual ERiftState GetRiftState_Implementation() const override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	FRiftCapabilities GetRiftCapabilities() const;
	virtual FRiftCapabilities GetRiftCapabilities_Implementation() const override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool PerformPhantomDodge(FVector Direction, bool bIsAerial);
	virtual bool PerformPhantomDodge_Implementation(FVector Direction, bool bIsAerial) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool InitiateRiftTether(const FVector& TargetLocation);
	virtual bool InitiateRiftTether_Implementation(const FVector& TargetLocation) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rift")
	bool ChainRiftTether();
	virtual bool ChainRiftTether_Implementation() override;

	// Current chain count
	UPROPERTY(BlueprintReadOnly, Category = "Rift")
	int32 CurrentChainCount = 0;

	// Counter rift properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift|Counter")
	float CounterRiftWindow = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift|Counter")
	float CounterRiftDistance = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift|Counter")
	float CounterRiftMomentumGain = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift|Counter")
	float CounterRiftStylePoints = 200.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Rift|Counter")
	bool bInCounterRiftWindow = false;

	UPROPERTY(BlueprintReadOnly, Category = "Rift|Counter")
	AActor* LastAttacker = nullptr;

	// Initiates a rift tether to a specific anchor
	UFUNCTION(BlueprintCallable, Category = "Rift")
	bool InitiateRiftTetherToAnchor(class ARiftAnchor* TargetAnchor);

	// Performs a counter rift if timing is correct
	UFUNCTION(BlueprintCallable, Category = "Rift")
	bool AttemptCounterRift();

	// Returns whether the component is currently in cooldown
	UFUNCTION(BlueprintPure, Category = "Rift")
	bool IsInCooldown() const { return CurrentRiftState == ERiftState::Cooldown; }

	// Returns whether the component is currently rifting
	UFUNCTION(BlueprintPure, Category = "Rift")
	bool IsRifting() const { return CurrentRiftState == ERiftState::Tethering || CurrentRiftState == ERiftState::Dodging; }

	// Delegate declaration for rift state changes
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRiftStateChanged);

	// Event triggered when a rift begins
	UPROPERTY(BlueprintAssignable, Category = "Rift")
	FOnRiftStateChanged OnRiftBegin;

	// Event triggered when a rift ends
	UPROPERTY(BlueprintAssignable, Category = "Rift")
	FOnRiftStateChanged OnRiftEnd;

	// Event triggered when a counter rift is performed
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCounterRiftPerformed, AActor*, CounteredActor);
	UPROPERTY(BlueprintAssignable, Category = "Rift")
	FOnCounterRiftPerformed OnCounterRiftPerformed;

	// Called when the owner takes damage, used for counter rift timing
	UFUNCTION(BlueprintCallable, Category = "Rift")
	void NotifyTakeDamage(float DamageAmount, AActor* DamageCauser);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Game event subsystem reference
	UPROPERTY()
	class UGameEventSubsystem* EventSystem;

	// Component locator reference
	UPROPERTY()
	class UComponentLocator* ComponentLocator;

	// Event listener handles
	TArray<int32> EventListenerHandles;

	// Event handlers
	UFUNCTION()
	void OnPlayerDamaged(const FGameEventData& EventData);

	// Broadcast events
	void BroadcastRiftPerformedEvent(FName RiftType, const FVector& StartLocation, const FVector& EndLocation);
	void BroadcastRiftStateChangedEvent(ERiftState NewState, ERiftState OldState);

	// Current rift state
	UPROPERTY(BlueprintReadOnly, Category = "Rift")
	ERiftState CurrentRiftState;

	// Current anchor for chaining
	UPROPERTY()
	class ARiftAnchor* CurrentAnchor;

	// Cooldown between rifts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	float RiftCooldown;

	// Base rift capabilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	FRiftCapabilities BaseCapabilities;

	// Current rift capabilities (modified by momentum)
	UPROPERTY(BlueprintReadOnly, Category = "Rift")
	FRiftCapabilities CurrentCapabilities;

	// Helper methods for accessing interfaces
	IMomentumInterface* GetMomentumInterface() const;
	IStyleInterface* GetStyleInterface() const;

	// Momentum gained from successful rift (moved from below)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	float RiftMomentumGain;



	// Remaining cooldown time
	UPROPERTY(BlueprintReadOnly, Category = "Rift")
	float RemainingCooldown;

	// Target location for current rift
	UPROPERTY(BlueprintReadOnly, Category = "Rift")
	FVector CurrentRiftTarget;



	// Character movement component reference
	UPROPERTY()
	UCharacterMovementComponent* CharacterMovement;

	// Character capsule component reference
	UPROPERTY()
	UCapsuleComponent* CapsuleComponent;

	// Calculate current rift capabilities based on momentum
	void UpdateRiftCapabilities();

	// Handle momentum changes when rifting
	void HandleRiftMomentumEffects(bool bSuccessfulRift);

	// Complete the current rift
	void CompleteRift(bool bSuccessful);

	// Start cooldown period
	void StartCooldown();

private:

	// Timer handle for rift cooldown
	FTimerHandle CooldownTimerHandle;

	// Timer handle for rift completion
	FTimerHandle RiftCompletionTimerHandle;

	// Timer handle for counter rift window
	FTimerHandle CounterRiftWindowTimerHandle;

	// Callback for when counter rift window ends
	void OnCounterRiftWindowEnd();

	// Callback for when cooldown ends
	void OnCooldownEnd();

	// Perform the actual rift teleportation
	void ExecuteRiftTeleport(const FVector& TargetLocation);

	// Check if a location is valid for rifting
	bool IsValidRiftLocation(const FVector& TargetLocation) const;
};