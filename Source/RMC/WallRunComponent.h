// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/WallRunInterface.h"
#include "Core/GameEventSystem.h"
#include "WallRunComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
class UCameraComponent;
class UGameEventSubsystem;
class IMomentumInterface;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UWallRunComponent : public UActorComponent, public IWallRunInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWallRunComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Component registration/unregistration
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	// Called when the component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Wall run parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunGravityScale = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float MaxWallRunDuration = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunMomentumGain = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallJumpOffForce = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallJumpUpForce = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float MinWallRunHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunTraceDistance = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunCameraTilt = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float CameraTiltInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float MinWallRunSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunControlMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	float WallRunCooldown = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun")
	TArray<TEnumAsByte<EObjectTypeQuery>> WallRunSurfaceTypes;

	// Wall run boost mechanics (TitanFall 2 inspired)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun|Advanced")
	float WallRunSpeedBoost = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun|Advanced")
	float WallRunAcceleration = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun|Advanced")
	float MaxWallRunSpeed = 1000.0f;

	// Wall run transitions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun|Advanced")
	float WallToWallJumpBoost = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun|Advanced")
	float CornerTransitionAssistDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallRun|Advanced")
	float CornerDetectionAngle = 45.0f;

	// Current state
	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	bool bIsWallRunning = false;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	float CurrentWallRunTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	bool bIsWallRunningRight = false;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	float CurrentCameraTilt = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun")
	float LastWallRunEndTime = 0.0f;

	// Advanced wall run state (TitanFall 2 inspired)
	UPROPERTY(BlueprintReadOnly, Category = "WallRun|State")
	float CurrentWallRunSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun|State")
	float LastWallNormalAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun|State")
	bool bIsTransitioningCorner = false;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun|State")
	bool bWasWallRunningPreviously = false;

	UPROPERTY(BlueprintReadOnly, Category = "WallRun|State")
	float TimeSinceLastWallRun = 0.0f;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "WallRun")
	bool TryStartWallRunInternal();

	UFUNCTION(BlueprintCallable, Category = "WallRun")
	void EndWallRunInternal(bool bJumped = false);

	UFUNCTION(BlueprintCallable, Category = "WallRun")
	bool PerformWallJumpInternal();

	UFUNCTION(BlueprintPure, Category = "WallRun")
	bool CanWallRunInternal() const;

	UFUNCTION(BlueprintPure, Category = "WallRun")
	float GetWallRunDirectionInternal() const;

	UFUNCTION(BlueprintCallable, Category = "WallRun")
	void UpdateCameraTiltInternal(float DeltaTime);

	// WallRunInterface implementation
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	bool IsWallRunning() const;
	virtual bool IsWallRunning_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	float GetWallRunDirection() const;
	virtual float GetWallRunDirection_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	FVector GetCurrentWallNormal() const;
	virtual FVector GetCurrentWallNormal_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	bool TryStartWallRun();
	virtual bool TryStartWallRun_Implementation() override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	void EndWallRun(bool bJumped);
	virtual void EndWallRun_Implementation(bool bJumped) override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	bool PerformWallJump();
	virtual bool PerformWallJump_Implementation() override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	bool CanWallRun() const;
	virtual bool CanWallRun_Implementation() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wall Run")
	void UpdateCameraTilt(float DeltaTime);
	virtual void UpdateCameraTilt_Implementation(float DeltaTime);

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallRunBegin, bool, bIsRightWall);
	UPROPERTY(BlueprintAssignable, Category = "WallRun|Events")
	FOnWallRunBegin OnWallRunBegin;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallRunEnd);
	UPROPERTY(BlueprintAssignable, Category = "WallRun|Events")
	FOnWallRunEnd OnWallRunEnd;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallJump);
	UPROPERTY(BlueprintAssignable, Category = "WallRun|Events")
	FOnWallJump OnWallJump;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// References to required components
	UPROPERTY()
	UCharacterMovementComponent* CharacterMovement;

	UPROPERTY()
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY()
	UCameraComponent* FollowCamera;

	// Event system reference
	UPROPERTY()
	UGameEventSubsystem* EventSystem;

	// Event listener handles
	TArray<int32> EventListenerHandles;

	// Wall detection
	FVector CurrentWallNormal;
	FVector LastWallNormal;

	// Helper functions
	bool DetectWall(FVector& OutWallNormal);
	bool IsWallRunSurface(const FHitResult& HitResult) const;
	void ApplyWallRunForces(float DeltaTime);
	void UpdateWallRunState(float DeltaTime);
	void HandleWallRunMomentum();

	// Advanced wall run helpers (TitanFall 2 inspired)
	void UpdateWallRunSpeed(float DeltaTime);
	bool DetectCorner(FVector& OutCornerNormal);
	void HandleCornerTransition(const FVector& CornerNormal);
	bool CheckWallToWallJump(FVector& OutNewWallNormal);
	void ApplyWallToWallJumpBoost(const FVector& NewWallNormal);

	// Check if extended wall run ability is available
	bool CheckExtendedWallRun() const;

	// Visual feedback for wall running based on momentum
	void UpdateWallRunVisuals();

	// Timer handle for cooldown
	FTimerHandle WallRunCooldownTimer;

	// Cooldown function
	void ResetWallRunCooldown();

	// Event broadcasting
	void BroadcastWallRunStartedEvent(bool bIsRightWall);
	void BroadcastWallRunEndedEvent(bool bJumped);
	void BroadcastWallJumpEvent();

	// Event handlers
	UFUNCTION()
	void OnMomentumChanged(const FGameEventData& EventData);

	// Interface helpers
	IMomentumInterface* GetMomentumInterface() const;
};