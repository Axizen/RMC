// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/GameEventSystem.h"
#include "AdvancedMovementComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
class UGameEventSubsystem;
class IMomentumInterface;

/**
 * AdvancedMovementComponent
 * 
 * Provides enhanced movement mechanics inspired by TitanFall 2, including:
 * - Sliding
 * - Bunny hopping
 * - Air control
 * - Momentum preservation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UAdvancedMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAdvancedMovementComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Component registration/unregistration
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	// Called when the component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Slide mechanics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideSpeed = 1000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideDuration = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideControlMultiplier = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideGravityScale = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideMinSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideFriction = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideDownhillAccelerationMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideUphillDecelerationMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slide")
	float SlideMomentumGain = 10.0f;

	// Bunny hop mechanics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|BunnyHop")
	float BunnyHopSpeedBoost = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|BunnyHop")
	float BunnyHopWindow = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|BunnyHop")
	float BunnyHopMaxSpeedMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|BunnyHop")
	float BunnyHopMomentumGain = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|BunnyHop")
	int32 MaxConsecutiveBunnyHops = 5;

	// Air control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|AirControl")
	float AirAcceleration = 4000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|AirControl")
	float AirMaxSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|AirControl")
	float AirControlBoostMultiplier = 1.2f;

	// Momentum preservation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Momentum")
	float MomentumPreservationRate = 0.9f;

	// Current state
	UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
	bool bIsSliding = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
	float CurrentSlideTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
	float LastSlideEndTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
	bool bIsBunnyHopping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
	float LastJumpTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
	int32 ConsecutiveBunnyHops = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Movement|State")
	float CurrentBunnyHopSpeed = 0.0f;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Movement|Slide")
	bool StartSlide();
	
	UFUNCTION(BlueprintCallable, Category = "Movement|Slide")
	void EndSlide();
	
	UFUNCTION(BlueprintCallable, Category = "Movement|BunnyHop")
	bool AttemptBunnyHop();

	UFUNCTION(BlueprintCallable, Category = "Movement|BunnyHop")
	void ResetBunnyHop();

	UFUNCTION(BlueprintPure, Category = "Movement|Slide")
	bool CanSlide() const;

	UFUNCTION(BlueprintPure, Category = "Movement|BunnyHop")
	bool CanBunnyHop() const;

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideBegin);
	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnSlideBegin OnSlideBegin;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideEnd);
	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnSlideEnd OnSlideEnd;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBunnyHop, int32, HopCount);
	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnBunnyHop OnBunnyHop;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// References to required components
	UPROPERTY()
	UCharacterMovementComponent* CharacterMovement;

	UPROPERTY()
	UCapsuleComponent* CapsuleComponent;

	// Event system reference
	UPROPERTY()
	UGameEventSubsystem* EventSystem;

	// Event listener handles
	TArray<int32> EventListenerHandles;

	// Original character values (to restore after sliding)
	float OriginalCapsuleHalfHeight;
	float OriginalGravityScale;
	float OriginalGroundFriction;
	float OriginalBrakingDeceleration;
	float OriginalMaxWalkSpeed;

	// Helper functions
	void UpdateSlideState(float DeltaTime);
	void ApplySlideForces(float DeltaTime);
	void HandleSlideMomentum();
	void UpdateBunnyHopState(float DeltaTime);
	void ApplyAirControl(float DeltaTime);
	bool IsOnDownhillSlope() const;
	float GetSlopeAngle() const;

	// Timer handle for cooldown
	FTimerHandle SlideCooldownTimer;

	// Cooldown function
	void ResetSlideCooldown();

	// Event broadcasting
	void BroadcastSlideBeginEvent();
	void BroadcastSlideEndEvent();
	void BroadcastBunnyHopEvent(int32 HopCount);

	// Event handlers
	UFUNCTION()
	void OnMomentumChanged(const FGameEventData& EventData);

	UFUNCTION()
	void OnJumpEvent(const FGameEventData& EventData);

	UFUNCTION()
	void OnLandedEvent(const FGameEventData& EventData);

	// Interface helpers
	IMomentumInterface* GetMomentumInterface() const;
};