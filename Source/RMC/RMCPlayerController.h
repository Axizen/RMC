// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RMCPlayerController.generated.h"

/**
 * Player controller for the RMC game with momentum-based movement system
 */
UCLASS(Blueprintable, BlueprintType, meta=(ShortTooltip="Player controller with momentum-based movement support."))
class RMC_API ARMCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Constructor
	ARMCPlayerController();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Camera settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraSmoothing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float SpeedFOVMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MaxFOVIncrease;

	// Input settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	float MouseSensitivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool bInvertYAxis;

	// Debug settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool bShowDebugInfo;

	// Blueprint callable functions
	/** Toggles debug information display */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleDebugInfo();

	/** Gets the current momentum value from the character */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCurrentMomentum() const;

	/** Gets the current momentum percentage from the character */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetMomentumPercent() const;

	/** Gets the dash cooldown percentage from the character's movement component */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetDashCooldownPercent() const;

	/** Gets whether the character is currently wall running */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsWallRunning() const;

	/** Gets whether the character is currently sliding */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSliding() const;

	/** Gets whether the character is currently dashing */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsDashing() const;

	/** Gets the character's current velocity magnitude */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCharacterSpeed() const;

	/** Gets the character's maximum possible speed */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetCharacterMaxSpeed() const;

	/** Gets the character's speed as a percentage of max speed */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetSpeedPercent() const;

protected:
	// Input handlers
	void OnToggleDebugInfo();

	// Helper functions
	UFUNCTION(BlueprintCallable, Category = "Camera", meta = (ToolTip = "Updates camera FOV based on character speed"))
	void UpdateCameraFOV(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Debug", meta = (ToolTip = "Displays debug information on screen"))
	void DisplayDebugInfo();

	// Default camera values
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ToolTip = "Default camera field of view"))
	float DefaultCameraFOV;

	// Getter/Setter functions
public:
	/** Sets the camera field of view */
	UFUNCTION(BlueprintCallable, Category = "Camera", meta = (ToolTip = "Sets the camera field of view"))
	void SetCameraFOV(float NewFOV) { CameraFOV = NewFOV; }

	/** Gets the current camera field of view */
	UFUNCTION(BlueprintPure, Category = "Camera", meta = (ToolTip = "Gets the current camera field of view"))
	float GetCameraFOV() const { return CameraFOV; }
};