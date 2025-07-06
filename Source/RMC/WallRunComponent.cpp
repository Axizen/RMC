// Fill out your copyright notice in the Description page of Project Settings.

#include "WallRunComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Core/ComponentLocator.h"
#include "Core/MomentumInterface.h"
#include "Core/GameEventSystem.h"
#include "MomentumComponent.h"

// Sets default values for this component's properties
UWallRunComponent::UWallRunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default surface types for wall running (WorldStatic by default)
	WallRunSurfaceTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

	// Initialize event listener handles array
	EventListenerHandles.Empty();
}

// Register with ComponentLocator
void UWallRunComponent::OnRegister()
{
	Super::OnRegister();

	// Register with the component locator
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->RegisterComponent(this, FName("WallRunComponent"));
		}
	}
}

// Unregister from ComponentLocator
void UWallRunComponent::OnUnregister()
{
	// Unregister from the component locator
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->UnregisterComponent(this);
		}
	}

	Super::OnUnregister();
}

// Clean up event listeners
void UWallRunComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove all event listeners
	if (EventSystem)
	{
		for (int32 Handle : EventListenerHandles)
		{
			EventSystem->RemoveEventListener(Handle);
		}
		EventListenerHandles.Empty();
	}

	Super::EndPlay(EndPlayReason);
}

// Called when the game starts
void UWallRunComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get references to required components
	AActor* Owner = GetOwner();
	if (Owner)
	{
		ACharacter* Character = Cast<ACharacter>(Owner);
		if (Character)
		{
			CharacterMovement = Character->GetCharacterMovement();
			CapsuleComponent = Character->GetCapsuleComponent();
			
			// Find camera component
			TArray<UCameraComponent*> Cameras;
			Character->GetComponents<UCameraComponent>(Cameras);
			if (Cameras.Num() > 0)
			{
				FollowCamera = Cameras[0];
			}
		}
	}

	// Get the event system
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance)
	{
		EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
		if (EventSystem)
		{
			// Register for momentum changed events
			int32 Handle = EventSystem->AddEventListenerWithObject(
				EGameEventType::MomentumChanged,
				this,
				FName("OnMomentumChanged")
			);
			EventListenerHandles.Add(Handle);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Failed to get GameEventSubsystem. Event-based communication will be disabled."), *GetName());
		}
	}

	// Check if we have a momentum interface
	if (!GetMomentumInterface())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No MomentumInterface found on owner. Wall run momentum integration will be disabled."), *GetName());
	}
}

// Called every frame
void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update wall run state if currently wall running
	if (bIsWallRunning)
	{
		UpdateWallRunState(DeltaTime);
		ApplyWallRunForces(DeltaTime);
	}
	// Track time since last wall run for smooth transitions
	else if (bWasWallRunningPreviously)
	{
		TimeSinceLastWallRun += DeltaTime;
		
		// Reset the flag after a certain time
		if (TimeSinceLastWallRun > 1.0f)
		{
			bWasWallRunningPreviously = false;
		}
	}

	// Update camera tilt
	UpdateCameraTiltInternal(DeltaTime);
}

bool UWallRunComponent::TryStartWallRunInternal()
{
	// Check if we can wall run
	if (!CanWallRunInternal())
	{
		return false;
	}

	// Detect wall
	FVector WallNormal;
	if (!DetectWall(WallNormal))
	{
		return false;
	}

	// Store wall normal
	CurrentWallNormal = WallNormal;
	LastWallNormal = WallNormal;

	// Determine if running on right or left wall
	FVector OwnerForward = GetOwner()->GetActorForwardVector();
	bIsWallRunningRight = FVector::DotProduct(FVector::CrossProduct(FVector::UpVector, CurrentWallNormal), OwnerForward) < 0.0f;

	// Start wall run
	bIsWallRunning = true;
	CurrentWallRunTime = 0.0f;
	
	// Initialize wall run speed
	if (bWasWallRunningPreviously && TimeSinceLastWallRun < 0.5f)
	{
		// If we were wall running recently, maintain speed for smoother transitions
		CurrentWallRunSpeed = FMath::Max(CurrentWallRunSpeed, WallRunSpeed);
	}
	else
	{
		// Start with base wall run speed
		CurrentWallRunSpeed = WallRunSpeed;
	}

	// Reset transition flags
	bIsTransitioningCorner = false;

	// Modify character movement
	if (CharacterMovement)
	{
		// Store original values to restore later
		CharacterMovement->GravityScale = WallRunGravityScale;
		CharacterMovement->AirControl = FMath::Max(CharacterMovement->AirControl, WallRunControlMultiplier);
		
		// Set movement mode to falling (wall run is a controlled fall)
		CharacterMovement->SetMovementMode(MOVE_Falling);
	}

	// Add momentum and handle momentum preservation
	HandleWallRunMomentum();

	// Initialize wall run visuals based on momentum
	UpdateWallRunVisuals();

	// Broadcast legacy event
	OnWallRunBegin.Broadcast(bIsWallRunningRight);

	// Broadcast event through event system
	BroadcastWallRunStartedEvent(bIsWallRunningRight);

	return true;
}

void UWallRunComponent::EndWallRunInternal(bool bJumped)
{
	// Only process if currently wall running
	if (!bIsWallRunning)
	{
		return;
	}

	// Store wall running state for transitions
	bWasWallRunningPreviously = true;
	TimeSinceLastWallRun = 0.0f;

	// Reset wall run state
	bIsWallRunning = false;
	CurrentWallRunTime = 0.0f;
	bIsTransitioningCorner = false;

	// Store end time for cooldown
	LastWallRunEndTime = GetWorld()->GetTimeSeconds();

	// Restore character movement properties
	if (CharacterMovement)
	{
		CharacterMovement->GravityScale = 1.0f;
		CharacterMovement->AirControl = 0.05f; // Default air control
	}

	// Resume momentum decay when wall running ends
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		// Get the UMomentumComponent directly
		UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
		if (MomentumComp)
		{
			// Resume momentum decay
			MomentumComp->ResumeMomentumDecay();
			
			// If not jumping off, trigger a smaller momentum pulse
			if (!bJumped)
			{
				MomentumComp->TriggerMomentumPulse(0.2f);
			}
		}
	}

	// Broadcast legacy event
	OnWallRunEnd.Broadcast();

	// Broadcast event through event system
	BroadcastWallRunEndedEvent(bJumped);

	// Reset wall run speed gradually over time
	// This allows us to maintain speed for a short time if we quickly transition to another wall
	FTimerHandle SpeedResetTimer;
	GetWorld()->GetTimerManager().SetTimer(
		SpeedResetTimer,
		[this]()
		{
			CurrentWallRunSpeed = WallRunSpeed;
			bWasWallRunningPreviously = false;
		},
		0.5f, // Reset speed after half a second if we don't start wall running again
		false
	);
}

bool UWallRunComponent::PerformWallJumpInternal()
{
	// Check if we're wall running
	if (!bIsWallRunning)
	{
		return false;
	}

	// Check if there's an opposite wall we can jump to
	FVector NewWallNormal;
	bool bCanWallToWallJump = CheckWallToWallJump(NewWallNormal);

	// Calculate jump direction
	FVector JumpDirection;
	
	if (bCanWallToWallJump)
	{
		// If we can wall-to-wall jump, direct jump towards the new wall
		JumpDirection = (CurrentWallNormal + NewWallNormal) * 0.5f;
		JumpDirection.Z = 0.0f;
		JumpDirection.Normalize();
	}
	else
	{
		// Standard wall jump (away from wall + up)
		JumpDirection = CurrentWallNormal;
		JumpDirection.Z = 0.0f;
		JumpDirection.Normalize();
	}

	// Get momentum interface for ability checks
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	UObject* MomentumObject = MomentumInterface ? Cast<UObject>(MomentumInterface) : nullptr;
	
	// Check if wall bounce ability is available
	bool bCanWallBounce = false;
	if (MomentumInterface && MomentumObject)
	{
		bCanWallBounce = MomentumInterface->Execute_IsPlatformingAbilityAvailable(MomentumObject, FName("WallBounce"));
	}

	// Apply force
	if (CharacterMovement)
	{
		// Calculate jump velocity with enhanced force if wall bounce is available
		float JumpOffForce = WallJumpOffForce;
		float JumpUpForce = WallJumpUpForce;
		
		// Apply wall bounce boost if available
		if (bCanWallBounce)
		{
			JumpOffForce *= 1.5f;
			JumpUpForce *= 1.3f;
		}
		
		// Apply wall-to-wall jump boost if available
		if (bCanWallToWallJump)
		{
			JumpOffForce += WallToWallJumpBoost;
			JumpUpForce *= 0.8f; // Less upward force for wall-to-wall jumps
		}
		
		FVector JumpVelocity = JumpDirection * JumpOffForce;
		JumpVelocity.Z = JumpUpForce;

		// Apply velocity
		CharacterMovement->Velocity = JumpVelocity;
		CharacterMovement->SetMovementMode(MOVE_Falling);

		// Reset jump count to allow double jumping after wall jump
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character)
		{
			Character->JumpCurrentCount = 0;
		}
	}

	// If this is a wall-to-wall jump, store the current wall normal for transition
	if (bCanWallToWallJump)
	{
		LastWallNormal = CurrentWallNormal;
	}

	// End wall run with jump flag
	EndWallRunInternal(true);

	// Broadcast legacy wall jump event
	OnWallJump.Broadcast();

	// Broadcast wall jump event through event system
	BroadcastWallJumpEvent();

	// Add momentum for wall jumping using interface
	if (MomentumInterface)
	{
		// Get the UMomentumComponent directly
		UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
		if (MomentumComp)
		{
			if (bCanWallToWallJump)
			{
				// Use a special momentum gain for wall-to-wall jumps
				MomentumComp->AddMomentum(WallRunMomentumGain * 2.0f, FName("WallToWallJump"));
				
				// Trigger a strong momentum pulse for visual feedback
				MomentumComp->TriggerMomentumPulse(0.9f);
			}
			else if (bCanWallBounce)
			{
				// Use the specialized wall bounce momentum method
				MomentumComp->AddWallBounceMomentum();
				
				// Trigger a stronger momentum pulse for visual feedback
				MomentumComp->TriggerMomentumPulse(0.8f);
			}
			else
			{
				// Use the standard wall run momentum gain with a multiplier
				MomentumComp->AddWallRunMomentum(0.0f);
				
				// Trigger a momentum pulse for visual feedback
				MomentumComp->TriggerMomentumPulse(0.5f);
			}
		}
		else
		{
			// Fallback to standard interface method
			MomentumInterface->AddMomentum(WallRunMomentumGain * 1.5f, FName("WallJump"));
		}
	}

	// If this is a wall-to-wall jump, try to start wall running on the new wall after a short delay
	if (bCanWallToWallJump)
	{
		FTimerHandle WallToWallJumpTimer;
		GetWorld()->GetTimerManager().SetTimer(
			WallToWallJumpTimer,
			[this, NewWallNormal]()
			{
				// Store the new wall normal
				CurrentWallNormal = NewWallNormal;
				
				// Try to start wall running on the new wall
				TryStartWallRunInternal();
			},
			0.1f, // Short delay to allow the character to move towards the new wall
			false
		);
	}

	return true;
}

bool UWallRunComponent::CanWallRunInternal() const
{
	// Check if required components exist
	if (!CharacterMovement || !CapsuleComponent)
	{
		return false;
	}

	// Check if in cooldown
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastWallRunEndTime < WallRunCooldown)
	{
		return false;
	}

	// Check if already wall running
	if (bIsWallRunning)
	{
		return false;
	}

	// Check if moving fast enough
	float HorizontalSpeed = FVector2D(CharacterMovement->Velocity.X, CharacterMovement->Velocity.Y).Size();
	if (HorizontalSpeed < MinWallRunSpeed)
	{
		return false;
	}

	// Check if in air
	if (CharacterMovement->IsMovingOnGround())
	{
		return false;
	}

	// Check if high enough from ground
	FHitResult GroundHit;
	FVector Start = GetOwner()->GetActorLocation();
	FVector End = Start - FVector(0, 0, MinWallRunHeight + CapsuleComponent->GetScaledCapsuleHalfHeight());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECC_Visibility, QueryParams))
	{
		// Too close to ground
		return false;
	}

	// Check if we have enough momentum to wall run
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		// Get the UMomentumComponent directly
		UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
		if (MomentumComp)
		{
			// Wall running requires at least 10 momentum
			const float RequiredMomentum = 10.0f;
			if (MomentumComp->GetCurrentMomentum() < RequiredMomentum)
			{
				// Not enough momentum to wall run
				return false;
			}
		}
		else
		{
			// Fallback to interface method
			const float RequiredMomentum = 10.0f;
			if (MomentumInterface->GetCurrentMomentum() < RequiredMomentum)
			{
				// Not enough momentum to wall run
				return false;
			}
		}
	}

	return true;
}

float UWallRunComponent::GetWallRunDirectionInternal() const
{
	// Return -1 for left wall, 1 for right wall, 0 for not wall running
	if (!bIsWallRunning)
	{
		return 0.0f;
	}

	return bIsWallRunningRight ? 1.0f : -1.0f;
}

void UWallRunComponent::UpdateCameraTiltInternal(float DeltaTime)
{
	// Skip if no camera
	if (!FollowCamera)
	{
		return;
	}

	// Calculate target tilt
	float TargetTilt = 0.0f;
	if (bIsWallRunning)
	{
		// Base tilt direction based on wall side
		TargetTilt = bIsWallRunningRight ? WallRunCameraTilt : -WallRunCameraTilt;
		
		// Enhance tilt based on momentum
		IMomentumInterface* MomentumInterface = GetMomentumInterface();
		if (MomentumInterface)
		{
			// Get the UMomentumComponent directly
			UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
			if (MomentumComp)
			{
				// Get momentum intensity (0-1) and enhance tilt by up to 50%
				float MomentumIntensity = MomentumComp->GetMomentumIntensity();
				float MomentumMultiplier = 1.0f + (0.5f * MomentumIntensity);
				TargetTilt *= MomentumMultiplier;
				
				// Also adjust interpolation speed based on momentum - higher momentum = faster response
				float AdjustedInterpSpeed = CameraTiltInterpSpeed * (1.0f + MomentumIntensity);
				
				// Interpolate current tilt to target with adjusted speed
				CurrentCameraTilt = FMath::FInterpTo(CurrentCameraTilt, TargetTilt, DeltaTime, AdjustedInterpSpeed);
			}
			else
			{
				// Fall back to standard interpolation if momentum interface is not available
				CurrentCameraTilt = FMath::FInterpTo(CurrentCameraTilt, TargetTilt, DeltaTime, CameraTiltInterpSpeed);
			}
		}
		else
		{
			// Fall back to standard interpolation if momentum interface is not available
			CurrentCameraTilt = FMath::FInterpTo(CurrentCameraTilt, TargetTilt, DeltaTime, CameraTiltInterpSpeed);
		}
	}
	else
	{
		// Not wall running, return to neutral
		CurrentCameraTilt = FMath::FInterpTo(CurrentCameraTilt, TargetTilt, DeltaTime, CameraTiltInterpSpeed);
	}

	// Apply rotation to camera
	FRotator CurrentRotation = FollowCamera->GetRelativeRotation();
	CurrentRotation.Roll = CurrentCameraTilt;
	FollowCamera->SetRelativeRotation(CurrentRotation);
}

bool UWallRunComponent::DetectWall(FVector& OutWallNormal)
{
	// Skip if no capsule component
	if (!CapsuleComponent || !GetWorld())
	{
		return false;
	}

	// Get character forward and right vectors
	FVector ForwardVector = GetOwner()->GetActorForwardVector();
	FVector RightVector = GetOwner()->GetActorRightVector();

	// Setup trace parameters
	FVector Start = GetOwner()->GetActorLocation();
	TArray<FHitResult> Hits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	// Check for walls on both sides
	bool bFoundWall = false;

	// Right side check
	FVector RightEnd = Start + RightVector * WallRunTraceDistance;
	TArray<FHitResult> RightHits;
	bool bHitRight = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		Start,
		RightEnd,
		15.0f,
		WallRunSurfaceTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		RightHits,
		true
	);

	// Left side check
	FVector LeftEnd = Start - RightVector * WallRunTraceDistance;
	TArray<FHitResult> LeftHits;
	bool bHitLeft = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		Start,
		LeftEnd,
		15.0f,
		WallRunSurfaceTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		LeftHits,
		true
	);

	// Process hits
	if (bHitRight && RightHits.Num() > 0)
	{
		// Find valid wall surface
		for (const FHitResult& Hit : RightHits)
		{
			if (IsWallRunSurface(Hit))
			{
				OutWallNormal = Hit.Normal;
				return true;
			}
		}
	}

	if (bHitLeft && LeftHits.Num() > 0)
	{
		// Find valid wall surface
		for (const FHitResult& Hit : LeftHits)
		{
			if (IsWallRunSurface(Hit))
			{
				OutWallNormal = Hit.Normal;
				return true;
			}
		}
	}

	return false;
}

bool UWallRunComponent::IsWallRunSurface(const FHitResult& HitResult) const
{
	// Check if surface is roughly vertical
	float DotWithUp = FVector::DotProduct(HitResult.Normal, FVector::UpVector);
	if (FMath::Abs(DotWithUp) > 0.3f) // Not vertical enough
	{
		return false;
	}

	// Additional checks can be added here (material type, etc.)

	return true;
}

void UWallRunComponent::ApplyWallRunForces(float DeltaTime)
{
	// Skip if no character movement
	if (!CharacterMovement || !GetOwner())
	{
		return;
	}

	// Calculate wall run direction (perpendicular to wall normal, along the wall)
	FVector WallRunDirection = FVector::CrossProduct(CurrentWallNormal, FVector::UpVector);
	
	// Ensure direction is correct based on which side of the wall we're on
	if ((bIsWallRunningRight && WallRunDirection.Y < 0) || (!bIsWallRunningRight && WallRunDirection.Y > 0))
	{
		WallRunDirection = -WallRunDirection;
	}

	WallRunDirection.Normalize();

	// Update wall run speed with acceleration and boost
	UpdateWallRunSpeed(DeltaTime);

	// Apply velocity along the wall
	FVector CurrentVelocity = CharacterMovement->Velocity;
	
	// Maintain height (counter gravity)
	CurrentVelocity.Z = FMath::Max(CurrentVelocity.Z, 0.0f);
	
	// Set horizontal velocity along wall using the current wall run speed
	CurrentVelocity.X = WallRunDirection.X * CurrentWallRunSpeed;
	CurrentVelocity.Y = WallRunDirection.Y * CurrentWallRunSpeed;
	
	// Apply velocity
	CharacterMovement->Velocity = CurrentVelocity;

	// Apply slight force towards wall to stay attached
	FVector ForceTowardsWall = -CurrentWallNormal * 50.0f;
	CharacterMovement->AddForce(ForceTowardsWall);

	// Check for corner transitions
	FVector CornerNormal;
	if (DetectCorner(CornerNormal))
	{
		HandleCornerTransition(CornerNormal);
	}

	// Check for wall-to-wall jumps if we're not already transitioning a corner
	if (!bIsTransitioningCorner)
	{
		FVector NewWallNormal;
		if (CheckWallToWallJump(NewWallNormal))
		{
			ApplyWallToWallJumpBoost(NewWallNormal);
		}
	}
}

void UWallRunComponent::UpdateWallRunState(float DeltaTime)
{
	// Increment wall run time
	CurrentWallRunTime += DeltaTime;

	// Check if we should use extended wall run duration
	float CurrentMaxWallRunDuration = MaxWallRunDuration;
	if (CheckExtendedWallRun())
	{
		CurrentMaxWallRunDuration *= 1.5f; // 50% longer wall run when ability is active
	}

	// Check if wall run should end due to time limit
	if (CurrentWallRunTime >= CurrentMaxWallRunDuration)
	{
		EndWallRunInternal(false);
		return;
	}

	// Check if still moving fast enough
	float HorizontalSpeed = FVector2D(CharacterMovement->Velocity.X, CharacterMovement->Velocity.Y).Size();
	if (HorizontalSpeed < MinWallRunSpeed * 0.8f) // Allow slight slowdown
	{
		EndWallRunInternal(false);
		return;
	}

	// Check if still near a wall (skip if we're in a corner transition)
	if (!bIsTransitioningCorner)
	{
		FVector NewWallNormal;
		if (!DetectWall(NewWallNormal))
		{
			// Before ending wall run, check if there's a corner we can transition to
			FVector CornerNormal;
			if (DetectCorner(CornerNormal))
			{
				HandleCornerTransition(CornerNormal);
			}
			else
			{
				EndWallRunInternal(false);
				return;
			}
		}
		else if (!NewWallNormal.Equals(CurrentWallNormal, 0.5f))
		{
			// Wall normal changed significantly (turned a corner)
			// Update wall normal
			CurrentWallNormal = NewWallNormal;
			
			// Determine if running on right or left wall
			FVector OwnerForward = GetOwner()->GetActorForwardVector();
			bool bNewIsWallRunningRight = FVector::DotProduct(FVector::CrossProduct(FVector::UpVector, CurrentWallNormal), OwnerForward) < 0.0f;
			
			// If wall side changed, broadcast event
			if (bNewIsWallRunningRight != bIsWallRunningRight)
			{
				bIsWallRunningRight = bNewIsWallRunningRight;
				OnWallRunBegin.Broadcast(bIsWallRunningRight);
			}
		}
	}

	// Check if landed on ground
	if (CharacterMovement->IsMovingOnGround())
	{
		EndWallRunInternal(false);
		return;
	}

	// Add continuous momentum while wall running
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		// Use the new AddWallRunMomentum method that takes duration as a parameter
		if (CurrentWallRunTime > 0.5f) // Only add continuous momentum after initial period
		{
			MomentumInterface->Execute_AddWallRunMomentum(Cast<UObject>(MomentumInterface), CurrentWallRunTime);
		}
	}

	// Update camera tilt based on wall run speed
	if (FollowCamera)
	{
		// Adjust FOV based on wall run speed
		float BaseFOV = 90.0f;
		float SpeedFactor = FMath::GetMappedRangeValueClamped(
			FVector2D(WallRunSpeed, MaxWallRunSpeed),
			FVector2D(0.0f, 10.0f),
			CurrentWallRunSpeed
		);
		
		// Apply FOV change gradually
		// Note: This is commented out as it would require access to the camera's base FOV
		// FollowCamera->FieldOfView = FMath::FInterpTo(FollowCamera->FieldOfView, BaseFOV + SpeedFactor, DeltaTime, 5.0f);
	}
}

void UWallRunComponent::HandleWallRunMomentum()
{
	// Add momentum for wall running using interface
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (!MomentumInterface)
	{
		return;
	}

	// Get the UMomentumComponent directly
	UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
	if (MomentumComp)
	{
		// Initial momentum boost when starting wall run
		MomentumComp->AddWallRunMomentum(0.0f);
		
		// Preserve momentum while wall running
		MomentumComp->PreserveMomentum(MaxWallRunDuration);
		
		// Trigger a momentum pulse for visual feedback
		MomentumComp->TriggerMomentumPulse(0.5f);
	}
	else
	{
		// Fallback to standard interface method if not a UMomentumComponent
		MomentumInterface->AddMomentum(WallRunMomentumGain, MOMENTUM_SOURCE_WALL_RUN);
	}
}

void UWallRunComponent::ResetWallRunCooldown()
{
	// Clear cooldown timer
	GetWorld()->GetTimerManager().ClearTimer(WallRunCooldownTimer);
}

// WallRunInterface implementation
bool UWallRunComponent::IsWallRunning_Implementation() const
{
	return bIsWallRunning;
}

float UWallRunComponent::GetWallRunDirection_Implementation() const
{
	return GetWallRunDirectionInternal();
}

FVector UWallRunComponent::GetCurrentWallNormal_Implementation() const
{
	return CurrentWallNormal;
}

bool UWallRunComponent::TryStartWallRun_Implementation()
{
	return TryStartWallRunInternal();
}

void UWallRunComponent::EndWallRun_Implementation(bool bJumped)
{
	EndWallRunInternal(bJumped);
}

bool UWallRunComponent::PerformWallJump_Implementation()
{
	return PerformWallJumpInternal();
}

bool UWallRunComponent::CanWallRun_Implementation() const
{
	// Use the internal implementation to avoid duplication
	return CanWallRunInternal();
}

void UWallRunComponent::UpdateCameraTilt_Implementation(float DeltaTime)
{
	// Use the internal implementation to avoid duplication
	UpdateCameraTiltInternal(DeltaTime);
}

// Event broadcasting methods
void UWallRunComponent::BroadcastWallRunStartedEvent(bool bIsRightWall)
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::WallRunStarted;
		EventData.Instigator = GetOwner();
		EventData.BoolValue = bIsRightWall;
		EventData.LocationValue = GetOwner()->GetActorLocation();
		EventData.NameValue = FName("WallRun");
		
		EventSystem->BroadcastEvent(EventData);
	}
}

void UWallRunComponent::BroadcastWallRunEndedEvent(bool bJumped)
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::WallRunEnded;
		EventData.Instigator = GetOwner();
		EventData.BoolValue = bJumped;
		EventData.LocationValue = GetOwner()->GetActorLocation();
		EventData.NameValue = bJumped ? FName("WallJump") : FName("WallRunEnd");
		
		EventSystem->BroadcastEvent(EventData);
	}
}

void UWallRunComponent::BroadcastWallJumpEvent()
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::WallRunEnded; // Use same event type but with different data
		EventData.Instigator = GetOwner();
		EventData.BoolValue = true; // Jumped = true
		EventData.LocationValue = GetOwner()->GetActorLocation();
		EventData.NameValue = FName("WallJump");
		
		EventSystem->BroadcastEvent(EventData);
	}
}

// Check if extended wall run ability is available
bool UWallRunComponent::CheckExtendedWallRun() const
{
	// Get momentum interface
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (!MomentumInterface)
	{
		return false;
	}

	// Get the UMomentumComponent directly
	UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
	if (MomentumComp)
	{
		// Check if the ExtendedWallRun ability is available
		return MomentumComp->IsPlatformingAbilityAvailable(FName("ExtendedWallRun"));
	}

	return false;
}

// Update wall run visuals based on momentum
void UWallRunComponent::UpdateWallRunVisuals()
{
	// Get momentum interface
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (!MomentumInterface)
	{
		return;
	}

	// Get the UMomentumComponent directly
	UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
	if (!MomentumComp)
	{
		return;
	}

	// Get momentum color and intensity for visual effects
	FLinearColor MomentumColor = MomentumComp->GetMomentumColor();
	float MomentumIntensity = MomentumComp->GetMomentumIntensity();

	// Here you would apply these values to any visual effects
	// For example, if you have a trail effect component:
	// if (TrailComponent)
	// {
	//     TrailComponent->SetColorParameter("Color", MomentumColor);
	//     TrailComponent->SetFloatParameter("Intensity", MomentumIntensity);
	// }

	// You could also adjust camera effects based on momentum intensity
	if (FollowCamera)
	{
		// Adjust FOV based on momentum intensity
		float BaseFOV = 90.0f;
		float MaxFOVIncrease = 15.0f;
		float TargetFOV = BaseFOV + (MaxFOVIncrease * MomentumIntensity);
		
		// You would need to smoothly interpolate to this value
		// FollowCamera->SetFieldOfView(TargetFOV);
	}
}

// Event handlers
void UWallRunComponent::OnMomentumChanged(const FGameEventData& EventData)
{
	// React to momentum changes
	if (bIsWallRunning)
	{
		// Update visuals when momentum changes during wall running
		UpdateWallRunVisuals();
		
		// Check if we now have access to extended wall run
		// This will be applied on the next tick in UpdateWallRunState
	}
}

// Interface helpers
IMomentumInterface* UWallRunComponent::GetMomentumInterface() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}
	
	// Check if the owner implements the interface
	if (Owner->Implements<UMomentumInterface>())
	{
		return Cast<IMomentumInterface>(Owner);
	}
	
	// If not, try to find a component that implements it
	UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		UActorComponent* Component = Locator->GetComponentByClass(UMomentumInterface::StaticClass());
		if (Component && Component->Implements<UMomentumInterface>())
		{
			return Cast<IMomentumInterface>(Component);
		}
	}
	
	// As a fallback, try to find any component that implements the interface
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component && Component->Implements<UMomentumInterface>())
		{
			return Cast<IMomentumInterface>(Component);
		}
	}
	
	return nullptr;
}
void UWallRunComponent::UpdateWallRunSpeed(float DeltaTime)
{
	// Initialize current wall run speed if this is the start of wall running
	if (CurrentWallRunTime <= DeltaTime)
	{
		// If we were wall running previously and quickly transitioned to another wall,
		// maintain speed for smoother transitions
		if (bWasWallRunningPreviously && TimeSinceLastWallRun < 0.5f)
		{
			// Keep previous speed but ensure it's at least the base wall run speed
			CurrentWallRunSpeed = FMath::Max(CurrentWallRunSpeed, WallRunSpeed);
		}
		else
		{
			// Start with base wall run speed
			CurrentWallRunSpeed = WallRunSpeed;
		}
	}

	// Calculate target speed based on wall run duration
	// The longer the wall run, the faster it gets (up to a maximum)
	float DurationFactor = FMath::Min(CurrentWallRunTime / 2.0f, 1.0f); // Max boost after 2 seconds
	float TargetSpeed = WallRunSpeed + (WallRunSpeedBoost * DurationFactor);

	// Apply momentum-based speed boost if available
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
		if (MomentumComp)
		{
			// Get momentum tier and add additional speed based on tier
			int32 MomentumTier = MomentumComp->GetMomentumTier();
			TargetSpeed += (MomentumTier * 50.0f); // 50 units per momentum tier

			// Additional boost if overcharged
			if (MomentumComp->IsOvercharged())
			{
				TargetSpeed *= 1.2f; // 20% speed boost when overcharged
			}
		}
	}

	// Cap at maximum wall run speed
	TargetSpeed = FMath::Min(TargetSpeed, MaxWallRunSpeed);

	// Smoothly accelerate to target speed
	CurrentWallRunSpeed = FMath::FInterpTo(CurrentWallRunSpeed, TargetSpeed, DeltaTime, WallRunAcceleration / 100.0f);
}
bool UWallRunComponent::DetectCorner(FVector& OutCornerNormal)
{
	// Skip if no capsule component
	if (!CapsuleComponent || !GetWorld())
	{
		return false;
	}

	// Get character forward vector
	FVector ForwardVector = GetOwner()->GetActorForwardVector();

	// Calculate the direction we're moving along the wall
	FVector WallRunDirection = FVector::CrossProduct(CurrentWallNormal, FVector::UpVector);
	if ((bIsWallRunningRight && WallRunDirection.Y < 0) || (!bIsWallRunningRight && WallRunDirection.Y > 0))
	{
		WallRunDirection = -WallRunDirection;
	}
	WallRunDirection.Normalize();

	// Setup trace parameters
	FVector Start = GetOwner()->GetActorLocation();
	FVector ForwardEnd = Start + WallRunDirection * CornerTransitionAssistDistance;

	// Trace forward along the wall to detect corners
	TArray<FHitResult> ForwardHits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	bool bHitForward = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		Start,
		ForwardEnd,
		15.0f,
		WallRunSurfaceTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		ForwardHits,
		true
	);

	// If we didn't hit anything, we might be approaching a corner
	if (!bHitForward || ForwardHits.Num() == 0)
	{
		// Trace at an angle to try to find the corner wall
		FVector AngleDirection;
		if (bIsWallRunningRight)
		{
			// If running on right wall, check right-forward
			AngleDirection = WallRunDirection.RotateAngleAxis(-CornerDetectionAngle, FVector::UpVector);
		}
		else
		{
			// If running on left wall, check left-forward
			AngleDirection = WallRunDirection.RotateAngleAxis(CornerDetectionAngle, FVector::UpVector);
		}

		FVector AngleEnd = Start + AngleDirection * CornerTransitionAssistDistance;
		TArray<FHitResult> AngleHits;

		bool bHitAngle = UKismetSystemLibrary::SphereTraceMultiForObjects(
			GetWorld(),
			Start,
			AngleEnd,
			15.0f,
			WallRunSurfaceTypes,
			false,
			ActorsToIgnore,
			EDrawDebugTrace::None,
			AngleHits,
			true
		);

		// If we hit something at an angle, we found a corner
		if (bHitAngle && AngleHits.Num() > 0)
		{
			// Find valid wall surface
			for (const FHitResult& Hit : AngleHits)
			{
				if (IsWallRunSurface(Hit))
				{
					OutCornerNormal = Hit.Normal;
					return true;
				}
			}
		}
	}

	return false;
}

void UWallRunComponent::HandleCornerTransition(const FVector& CornerNormal)
{
	// Skip if already transitioning
	if (bIsTransitioningCorner)
	{
		return;
	}

	// Calculate angle between current wall and corner wall
	float WallAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurrentWallNormal, CornerNormal)));

	// Store the angle for reference
	LastWallNormalAngle = WallAngle;

	// If the angle is significant, this is a corner that needs transition assistance
	if (WallAngle > 30.0f && WallAngle < 120.0f)
	{
		bIsTransitioningCorner = true;

		// Update wall normal to the new corner
		CurrentWallNormal = CornerNormal;

		// Determine if running on right or left wall based on new normal
		FVector OwnerForward = GetOwner()->GetActorForwardVector();
		bool bNewIsWallRunningRight = FVector::DotProduct(FVector::CrossProduct(FVector::UpVector, CurrentWallNormal), OwnerForward) < 0.0f;

		// If wall side changed, broadcast event
		if (bNewIsWallRunningRight != bIsWallRunningRight)
		{
			bIsWallRunningRight = bNewIsWallRunningRight;
			OnWallRunBegin.Broadcast(bIsWallRunningRight);
		}

		// Apply a slight boost to help navigate the corner
		CurrentWallRunSpeed *= 1.1f; // 10% speed boost for corner transitions

		// Reset transition flag after a short delay
		FTimerHandle CornerTransitionTimer;
		GetWorld()->GetTimerManager().SetTimer(
			CornerTransitionTimer,
			[this]()
			{
				bIsTransitioningCorner = false;
			},
			0.2f, // Short delay to prevent multiple transitions at once
			false
		);
	}
}
bool UWallRunComponent::CheckWallToWallJump(FVector& OutNewWallNormal)
{
	// Skip if no capsule component
	if (!CapsuleComponent || !GetWorld())
	{
		return false;
	}

	// Only check for wall-to-wall jumps if we're already wall running
	if (!bIsWallRunning)
	{
		return false;
	}

	// Get the opposite direction from current wall
	FVector OppositeDirection = CurrentWallNormal;
	OppositeDirection.Z = 0.0f;
	OppositeDirection.Normalize();

	// Setup trace parameters
	FVector Start = GetOwner()->GetActorLocation();
	FVector End = Start + OppositeDirection * (WallRunTraceDistance * 2.0f); // Check further for opposite walls

	TArray<FHitResult> Hits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		Start,
		End,
		15.0f,
		WallRunSurfaceTypes,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		Hits,
		true
	);

	// Check if we found an opposite wall
	if (bHit && Hits.Num() > 0)
	{
		// Find valid wall surface
		for (const FHitResult& Hit : Hits)
		{
			if (IsWallRunSurface(Hit))
			{
				// Check if this is a different wall (not the one we're currently on)
				if (!Hit.Normal.Equals(CurrentWallNormal, 0.7f))
				{
					OutNewWallNormal = Hit.Normal;
					return true;
				}
			}
		}
	}

	return false;
}

void UWallRunComponent::ApplyWallToWallJumpBoost(const FVector& NewWallNormal)
{
	// Skip if no character movement
	if (!CharacterMovement)
	{
		return;
	}

	// Calculate jump direction (away from current wall, towards new wall)
	FVector JumpDirection = CurrentWallNormal + NewWallNormal;
	JumpDirection.Z = 0.0f;
	JumpDirection.Normalize();

	// Store the current wall normal as the last wall normal
	LastWallNormal = CurrentWallNormal;

	// Update to the new wall normal
	CurrentWallNormal = NewWallNormal;

	// Determine if running on right or left wall based on new normal
	FVector OwnerForward = GetOwner()->GetActorForwardVector();
	bool bNewIsWallRunningRight = FVector::DotProduct(FVector::CrossProduct(FVector::UpVector, CurrentWallNormal), OwnerForward) < 0.0f;

	// Update wall running side
	bIsWallRunningRight = bNewIsWallRunningRight;

	// Apply wall-to-wall jump boost
	FVector BoostVelocity = JumpDirection * WallToWallJumpBoost;
	CharacterMovement->Velocity += BoostVelocity;

	// Preserve or increase current wall run speed for smoother transitions
	CurrentWallRunSpeed = FMath::Max(CurrentWallRunSpeed, WallRunSpeed * 1.2f);

	// Add momentum for wall-to-wall jumps
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		// Get the UMomentumComponent directly
		UMomentumComponent* MomentumComp = Cast<UMomentumComponent>(Cast<UObject>(MomentumInterface));
		if (MomentumComp)
		{
			// Add momentum for wall-to-wall transition
			MomentumComp->AddMomentum(WallRunMomentumGain * 1.5f, FName("WallToWallJump"));
			
			// Trigger a momentum pulse for visual feedback
			MomentumComp->TriggerMomentumPulse(0.7f);
		}
		else
		{
			// Fallback to standard interface method
			MomentumInterface->AddMomentum(WallRunMomentumGain * 1.5f, FName("WallToWallJump"));
		}
	}

	// Broadcast wall run begin event with new wall side
	OnWallRunBegin.Broadcast(bIsWallRunningRight);

	// Broadcast event through event system
	BroadcastWallRunStartedEvent(bIsWallRunningRight);
}
