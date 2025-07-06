// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/DoubleJumpInterface.h"
#include "Core/GameEventSystem.h"
#include "DoubleJumpComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
class UGameEventSubsystem;
class IMomentumInterface;
class UMomentumComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UDoubleJumpComponent : public UActorComponent, public IDoubleJumpInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDoubleJumpComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Component registration/unregistration
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	// Called when the component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Double jump parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float DoubleJumpForce = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float DoubleJumpHorizontalBoost = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	int32 MaxDoubleJumps = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float DoubleJumpCooldown = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float MinMomentumForDoubleJump = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float MomentumScaledJumpBoost = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float MomentumTier1JumpBonus = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float MomentumTier2JumpBonus = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float MomentumTier3JumpBonus = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
	float DoubleJumpMomentumGain = 5.0f;

	// Current state
	UPROPERTY(BlueprintReadOnly, Category = "Double Jump")
	int32 CurrentDoubleJumpCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Double Jump")
	bool bIsDoubleJumping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Double Jump")
	float LastDoubleJumpTime = 0.0f;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Double Jump")
	bool CanDoubleJumpInternal() const;

	UFUNCTION(BlueprintCallable, Category = "Double Jump")
	bool PerformDoubleJumpInternal();

	UFUNCTION(BlueprintCallable, Category = "Double Jump")
	void ResetDoubleJumpCountInternal();

	UFUNCTION(BlueprintCallable, Category = "Double Jump")
	float GetDoubleJumpForceWithMomentum() const;

	UFUNCTION(BlueprintCallable, Category = "Double Jump")
	int32 GetMaxDoubleJumpsWithMomentum() const;

	// DoubleJumpInterface implementation
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	bool CanDoubleJump() const;
	virtual bool CanDoubleJump_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	bool PerformDoubleJump();
	virtual bool PerformDoubleJump_Implementation() override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	int32 GetDoubleJumpCount() const;
	virtual int32 GetDoubleJumpCount_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	int32 GetMaxDoubleJumps() const;
	virtual int32 GetMaxDoubleJumps_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	float GetDoubleJumpForce() const;
	virtual float GetDoubleJumpForce_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	void ResetDoubleJumpCount();
	virtual void ResetDoubleJumpCount_Implementation() override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Double Jump")
	bool IsDoubleJumping() const;
	virtual bool IsDoubleJumping_Implementation() const override;

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoubleJumpPerformed);
	UPROPERTY(BlueprintAssignable, Category = "Double Jump|Events")
	FOnDoubleJumpPerformed OnDoubleJumpPerformed;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoubleJumpReset);
	UPROPERTY(BlueprintAssignable, Category = "Double Jump|Events")
	FOnDoubleJumpReset OnDoubleJumpReset;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxDoubleJumpsChanged, int32, NewMaxDoubleJumps);
	UPROPERTY(BlueprintAssignable, Category = "Double Jump|Events")
	FOnMaxDoubleJumpsChanged OnMaxDoubleJumpsChanged;

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

	// Helper functions
	void HandleDoubleJumpMomentum();
	void UpdateDoubleJumpVisuals();
	void BroadcastDoubleJumpEvent();
	void BroadcastDoubleJumpResetEvent();

	// Timer handle for cooldown
	FTimerHandle DoubleJumpCooldownTimer;

	// Cooldown function
	UFUNCTION()
	void ResetDoubleJumpCooldown();

	// Event handlers
	UFUNCTION()
	void OnMomentumChanged(const FGameEventData& EventData);

	UFUNCTION()
	void OnLanded(const FGameEventData& EventData);

	// Interface helpers
	IMomentumInterface* GetMomentumInterface() const;
	UMomentumComponent* GetMomentumComponent() const;
};