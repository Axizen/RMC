// Fill out your copyright notice in the Description page of Project Settings.

#include "RMCPlayerController.h"
#include "RMCCharacter.h"
#include "Components/Movement/RMCMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"

ARMCPlayerController::ARMCPlayerController()
{
	// Set default values
	CameraSmoothing = 5.0f;
	CameraFOV = 90.0f;
	SpeedFOVMultiplier = 0.05f;
	MaxFOVIncrease = 15.0f;

	MouseSensitivity = 1.0f;
	bInvertYAxis = false;

	bShowDebugInfo = false;
}

void ARMCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Store default camera FOV
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter && RMCCharacter->FollowCamera)
	{
		DefaultCameraFOV = RMCCharacter->FollowCamera->FieldOfView;
		CameraFOV = DefaultCameraFOV;
	}
}

void ARMCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind debug toggle
	InputComponent->BindAction("ToggleDebugInfo", IE_Pressed, this, &ARMCPlayerController::OnToggleDebugInfo);
}

void ARMCPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update camera FOV based on speed
	UpdateCameraFOV(DeltaTime);

	// Display debug info if enabled
	if (bShowDebugInfo)
	{
		DisplayDebugInfo();
	}
}

void ARMCPlayerController::ToggleDebugInfo()
{
	bShowDebugInfo = !bShowDebugInfo;
}

float ARMCPlayerController::GetCurrentMomentum() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (MovementComponent)
		{
			return MovementComponent->CurrentMomentum;
		}
	}
	return 0.0f;
}

float ARMCPlayerController::GetMomentumPercent() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (MovementComponent)
		{
			return MovementComponent->GetMomentumPercent();
		}
	}
	return 0.0f;
}

float ARMCPlayerController::GetDashCooldownPercent() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (MovementComponent)
		{
			return MovementComponent->GetDashCooldownPercent();
		}
	}
	return 0.0f;
}

bool ARMCPlayerController::IsWallRunning() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (MovementComponent)
		{
			return MovementComponent->bIsWallRunning;
		}
	}
	return false;
}

bool ARMCPlayerController::IsSliding() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (MovementComponent)
		{
			return MovementComponent->bIsSliding;
		}
	}
	return false;
}

bool ARMCPlayerController::IsDashing() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (MovementComponent)
		{
			return MovementComponent->bIsDashing;
		}
	}
	return false;
}

float ARMCPlayerController::GetCharacterSpeed() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		return RMCCharacter->GetVelocity().Size();
	}
	return 0.0f;
}

float ARMCPlayerController::GetCharacterMaxSpeed() const
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter)
	{
		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (MovementComponent)
		{
			return MovementComponent->GetMaxSpeed();
		}
	}
	return 0.0f;
}

float ARMCPlayerController::GetSpeedPercent() const
{
	float MaxSpeed = GetCharacterMaxSpeed();
	if (MaxSpeed > 0.0f)
	{
		return FMath::Clamp(GetCharacterSpeed() / MaxSpeed, 0.0f, 1.0f);
	}
	return 0.0f;
}

void ARMCPlayerController::OnToggleDebugInfo()
{
	ToggleDebugInfo();
}

void ARMCPlayerController::UpdateCameraFOV(float DeltaTime)
{
	ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
	if (RMCCharacter && RMCCharacter->FollowCamera)
	{
		// Calculate target FOV based on speed
		float SpeedPercent = GetSpeedPercent();
		float TargetFOV = DefaultCameraFOV + (SpeedPercent * SpeedFOVMultiplier * MaxFOVIncrease);

		// Smoothly interpolate to target FOV
		CameraFOV = FMath::FInterpTo(RMCCharacter->FollowCamera->FieldOfView, TargetFOV, DeltaTime, CameraSmoothing);
		RMCCharacter->FollowCamera->FieldOfView = CameraFOV;
	}
}

void ARMCPlayerController::DisplayDebugInfo()
{
	if (GEngine)
	{
		// Get character and movement component
		ARMCCharacter* RMCCharacter = Cast<ARMCCharacter>(GetPawn());
		if (!RMCCharacter) return;

		URMCMovementComponent* MovementComponent = RMCCharacter->GetRMCMovementComponent();
		if (!MovementComponent) return;

		// Create debug strings
		FString DebugInfo;
		DebugInfo += FString::Printf(TEXT("Speed: %.2f / %.2f (%.0f%%)"), 
			GetCharacterSpeed(), 
			GetCharacterMaxSpeed(), 
			GetSpeedPercent() * 100.0f);

		DebugInfo += FString::Printf(TEXT("\nMomentum: %.2f / %.2f (%.0f%%)"), 
			MovementComponent->CurrentMomentum, 
			MovementComponent->MaxMomentum, 
			GetMomentumPercent() * 100.0f);

		DebugInfo += FString::Printf(TEXT("\nDash Cooldown: %.0f%%"), 
			GetDashCooldownPercent() * 100.0f);

		DebugInfo += FString::Printf(TEXT("\nMovement State: %s%s%s%s"), 
			MovementComponent->bIsWallRunning ? TEXT("Wall Running ") : TEXT(""), 
			MovementComponent->bIsSliding ? TEXT("Sliding ") : TEXT(""), 
			MovementComponent->bIsDashing ? TEXT("Dashing ") : TEXT(""), 
			MovementComponent->IsFalling() ? TEXT("In Air") : TEXT("Grounded"));

		// Display debug info on screen
		GEngine->AddOnScreenDebugMessage(0, 0.0f, FColor::Yellow, DebugInfo);
	}
}