// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/Movement/RMCMovementComponent.h"
#include "RMCCharacter.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(ShortTooltip="Character class with momentum-based movement system."))
class RMC_API ARMCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARMCCharacter(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Returns the custom movement component
	URMCMovementComponent* GetRMCMovementComponent() const;

	// Returns Camera Boom
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// Returns Follow Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Camera settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float WallRunCameraTilt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float WallRunCameraTiltSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float SlideCameraLowerOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float SlideCameraSpeed;

	// Animation properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool bIsWallRunningLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool bIsWallRunningRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool bRotateRootBoneWithController;

	// Movement Input Functions
	/** Called for forwards/backward input */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveForward(float Value);

	/** Called for side to side input */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveRight(float Value);

	/** Called for looking up/down input */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LookUp(float Value);

	/** Called for turning input */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Turn(float Value);

	/** Called via input to turn at a given rate */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void TurnAtRate(float Rate);

	/** Called via input to look up/down at a given rate */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LookUpAtRate(float Rate);

	// Momentum-based movement actions
	/** Initiates a jump or double jump if in air */
	UFUNCTION(BlueprintCallable, Category = "Movement|Actions")
	virtual void OnJumpActionPressed();

	/** Ends jump action */
	UFUNCTION(BlueprintCallable, Category = "Movement|Actions")
	virtual void OnJumpActionReleased();

	/** Initiates a dash in the current movement direction */
	UFUNCTION(BlueprintCallable, Category = "Movement|Actions")
	virtual void OnDashActionPressed();

	/** Initiates a slide if moving and on ground */
	UFUNCTION(BlueprintCallable, Category = "Movement|Actions")
	virtual void OnSlideActionPressed();

	/** Ends slide action */
	UFUNCTION(BlueprintCallable, Category = "Movement|Actions")
	virtual void OnSlideActionReleased();

	/** Attempts to start wall running */
	UFUNCTION(BlueprintCallable, Category = "Movement|Actions")
	virtual void TryWallRun();

	// Blueprint native events for animation and effects
	/** Called when character starts wall running */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when character starts wall running"))
	void OnWallRunBegin(const FVector& WallNormal);
	virtual void OnWallRunBegin_Implementation(const FVector& WallNormal);

	/** Called when character stops wall running */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when character stops wall running"))
	void OnWallRunEnd();
	virtual void OnWallRunEnd_Implementation();

	/** Called when character starts sliding */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when character starts sliding"))
	void OnSlideBegin();
	virtual void OnSlideBegin_Implementation();

	/** Called when character stops sliding */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when character stops sliding"))
	void OnSlideEnd();
	virtual void OnSlideEnd_Implementation();

	/** Called when character performs a dash */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when character performs a dash"))
	void OnDashBegin(const FVector& DashDirection);
	virtual void OnDashBegin_Implementation(const FVector& DashDirection);

	/** Called when character finishes a dash */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when character finishes a dash"))
	void OnDashEnd();
	virtual void OnDashEnd_Implementation();

	/** Called when character performs a double jump */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when character performs a double jump"))
	void OnDoubleJump();
	virtual void OnDoubleJump_Implementation();

	/** Called when momentum changes */
	UFUNCTION(BlueprintNativeEvent, Category = "Movement|Events", meta = (ToolTip = "Called when momentum changes"))
	void OnMomentumChanged(float NewMomentum);
	virtual void OnMomentumChanged_Implementation(float NewMomentum);

	// Debug helper functions
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleDebugMode();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DebugWallRunning();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void EnhanceWallRunning(float SpeedMultiplier = 1.2f);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	FString GetDebugInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugHelpers(float Duration = 0.1f);

	// Debug properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bDebugModeEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bEnhanceWallRunning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	float WallRunSpeedMultiplier;

protected:
	// Movement component event handlers
	UFUNCTION()
	void HandleWallRunBegin(const FVector& WallNormal);

	UFUNCTION()
	void HandleWallRunEnd();

	UFUNCTION()
	void HandleSlideBegin();

	UFUNCTION()
	void HandleSlideEnd();

	UFUNCTION()
	void HandleDashBegin(const FVector& DashDirection);

	UFUNCTION()
	void HandleDashEnd();

	UFUNCTION()
	void HandleMomentumChanged(float NewMomentum);

	// Getter functions for input values
	/** Get the current forward input value */
	UFUNCTION(BlueprintPure, Category = "Movement|Input", meta = (ToolTip = "Get the current forward input value"))
	float GetForwardInputValue() const { return ForwardInputValue; }

	/** Get the current right input value */
	UFUNCTION(BlueprintPure, Category = "Movement|Input", meta = (ToolTip = "Get the current right input value"))
	float GetRightInputValue() const { return RightInputValue; }

	// Camera control functions
	/** Updates camera tilt during wall running */
	UFUNCTION(BlueprintCallable, Category = "Camera", meta = (ToolTip = "Updates camera tilt during wall running"))
	void UpdateCameraDuringWallRun(float DeltaTime);

	/** Updates camera position during sliding */
	UFUNCTION(BlueprintCallable, Category = "Camera", meta = (ToolTip = "Updates camera position during sliding"))
	void UpdateCameraDuringSlide(float DeltaTime);

	/** Resets camera to default position and rotation */
	UFUNCTION(BlueprintCallable, Category = "Camera", meta = (ToolTip = "Resets camera to default position and rotation"))
	void ResetCameraToDefault(float DeltaTime);

	// Movement input values
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement|Input", meta = (ToolTip = "Current forward movement input value"))
	float ForwardInputValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement|Input", meta = (ToolTip = "Current right movement input value"))
	float RightInputValue;

	// Default camera values
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "Default camera boom arm length"))
	float DefaultCameraBoomLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "Default camera relative location"))
	FVector DefaultCameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ToolTip = "Default camera relative rotation"))
	FRotator DefaultCameraRotation;

	// Timer handles
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement|Timers", meta = (ToolTip = "Timer for checking wall run conditions"))
	FTimerHandle TimerHandle_CheckWallRun;
};