// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Core/RailGrindInterface.h"
#include "../Core/GameEventSystem.h"
#include "RailSplineActor.h"
#include "RailGrindComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
class UCameraComponent;
class UGameEventSubsystem;
class IMomentumInterface;
class IStyleInterface;

// Struct for rail trick data
USTRUCT(BlueprintType)
struct FRailTrickData
{
	GENERATED_BODY()

	// Name of the trick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	FText DisplayName;

	// Style points awarded for this trick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	float StylePoints;

	// Momentum gained from this trick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	float MomentumGain;

	// Animation to play for this trick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	UAnimMontage* TrickAnimation;

	// Minimum momentum required to perform this trick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	float MinimumMomentum;

	// Cooldown time for this trick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	float Cooldown;

	// Whether this trick requires a trick section on the rail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	bool bRequiresTrickSection;

	// Input combination required for this trick
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Trick")
	TArray<FName> InputSequence;

	// Default constructor
	FRailTrickData()
		: DisplayName(FText::FromString("Rail Trick"))
		, StylePoints(100.0f)
		, MomentumGain(10.0f)
		, TrickAnimation(nullptr)
		, MinimumMomentum(20.0f)
		, Cooldown(1.0f)
		, bRequiresTrickSection(false)
	{
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API URailGrindComponent : public UActorComponent, public IRailGrindInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URailGrindComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Component registration/unregistration
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	// Called when the component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Rail grind parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float BaseRailGrindSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailGrindGravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float MaxRailGrindDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailGrindMomentumGain;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailJumpForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailJumpUpForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailDetectionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailSwitchDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailCameraTilt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float CameraTiltInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float MinMomentumToGrind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float RailGrindCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	float StylePointsPerSecond;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind")
	TArray<TEnumAsByte<EObjectTypeQuery>> RailObjectTypes;

	// Rail tricks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail Grind|Tricks")
	TMap<FName, FRailTrickData> AvailableTricks;

	// Current state
	UPROPERTY(BlueprintReadOnly, Category = "Rail Grind")
	bool bIsRailGrinding;

	UPROPERTY(BlueprintReadOnly, Category = "Rail Grind")
	float CurrentRailGrindTime;

	UPROPERTY(BlueprintReadOnly, Category = "Rail Grind")
	float CurrentRailDistance;

	UPROPERTY(BlueprintReadOnly, Category = "Rail Grind")
	float CurrentRailPosition;

	UPROPERTY(BlueprintReadOnly, Category = "Rail Grind")
	float CurrentCameraTilt;

	UPROPERTY(BlueprintReadOnly, Category = "Rail Grind")
	float LastRailGrindEndTime;

	UPROPERTY(BlueprintReadOnly, Category = "Rail Grind")
	ARailSplineActor* CurrentRail;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	bool TryStartRailGrindInternal();

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	void EndRailGrindInternal(bool bJumped = false);

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	bool PerformRailJumpInternal();

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	bool PerformRailTrickInternal(FName TrickName);

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	bool SwitchRailInternal(int32 Direction);

	UFUNCTION(BlueprintPure, Category = "Rail Grind")
	bool CanRailGrind() const;

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	void UpdateCameraTilt(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	void UpdateRailPosition(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	ARailSplineActor* DetectRail() const;

	UFUNCTION(BlueprintCallable, Category = "Rail Grind")
	TArray<ARailSplineActor*> DetectNearbyRails(float MaxDistance) const;

	// RailGrindInterface implementation
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool IsRailGrinding() const;
	virtual bool IsRailGrinding_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetRailGrindSpeed() const;
	virtual float GetRailGrindSpeed_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetBaseRailGrindSpeed() const;
	virtual float GetBaseRailGrindSpeed_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetCurrentRailGrindTime() const;
	virtual float GetCurrentRailGrindTime_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool CanStartRailGrind() const;
	virtual bool CanStartRailGrind_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool TryStartRailGrind();
	virtual bool TryStartRailGrind_Implementation() override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	void EndRailGrind(bool bJumped);
	virtual void EndRailGrind_Implementation(bool bJumped) override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool PerformRailJump();
	virtual bool PerformRailJump_Implementation() override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool PerformRailTrick(FName TrickName);
	virtual bool PerformRailTrick_Implementation(FName TrickName) override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	bool SwitchRail(int32 Direction);
	virtual bool SwitchRail_Implementation(int32 Direction) override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	ARailSplineActor* GetCurrentRail() const;
	virtual ARailSplineActor* GetCurrentRail_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	float GetRailPosition() const;
	virtual float GetRailPosition_Implementation() const override;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Rail Grind")
	FVector GetRailDirection() const;
	virtual FVector GetRailDirection_Implementation() const override;

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRailGrindBegin, ARailSplineActor*, Rail);
	UPROPERTY(BlueprintAssignable, Category = "Rail Grind|Events")
	FOnRailGrindBegin OnRailGrindBegin;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRailGrindEnd);
	UPROPERTY(BlueprintAssignable, Category = "Rail Grind|Events")
	FOnRailGrindEnd OnRailGrindEnd;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRailJump);
	UPROPERTY(BlueprintAssignable, Category = "Rail Grind|Events")
	FOnRailJump OnRailJump;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRailTrick, FName, TrickName);
	UPROPERTY(BlueprintAssignable, Category = "Rail Grind|Events")
	FOnRailTrick OnRailTrick;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRailSwitch, ARailSplineActor*, NewRail, ARailSplineActor*, PreviousRail);
	UPROPERTY(BlueprintAssignable, Category = "Rail Grind|Events")
	FOnRailSwitch OnRailSwitch;

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

	// Rail detection
	FVector LastRailNormal;

	// Helper functions
	bool IsRailSurfaceValid(const FHitResult& HitResult) const;
	void ApplyRailGrindForces(float DeltaTime);
	void UpdateRailGrindState(float DeltaTime);
	void HandleRailGrindMomentum(float DeltaTime);
	void UpdateRailGrindVisuals(float DeltaTime);

	// Timer handle for cooldown
	FTimerHandle RailGrindCooldownTimer;

	// Cooldown function
	void ResetRailGrindCooldown();

	// Event broadcasting
	void BroadcastRailGrindStartedEvent(ARailSplineActor* Rail);
	void BroadcastRailGrindEndedEvent(bool bJumped);
	void BroadcastRailJumpEvent();
	void BroadcastRailTrickEvent(FName TrickName);
	void BroadcastRailSwitchEvent(ARailSplineActor* NewRail, ARailSplineActor* PreviousRail);

	// Event handlers
	UFUNCTION()
	void OnMomentumChanged(const FGameEventData& EventData);

	// Interface helpers
	IMomentumInterface* GetMomentumInterface() const;
	IStyleInterface* GetStyleInterface() const;

	// Trick system
	TArray<FName> InputBuffer;
	float InputBufferTimeWindow;
	TMap<FName, float> TrickCooldowns;

	UFUNCTION()
	void ProcessInputBuffer();

	UFUNCTION()
	void ClearInputBuffer();

	UFUNCTION()
	bool CheckTrickSequence(FName TrickName);

	UFUNCTION()
	void UpdateTrickCooldowns(float DeltaTime);

	// Momentum-based enhancements
	bool CheckEnhancedRailGrind() const;
	bool CheckEnhancedRailJump() const;
	float GetMomentumSpeedBonus() const;

	// Style point accumulation
	float StylePointAccumulator;
	void AccumulateStylePoints(float DeltaTime);
	void AwardStylePoints();
};