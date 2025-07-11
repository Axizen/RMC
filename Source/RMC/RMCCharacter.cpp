// Fill out your copyright notice in the Description page of Project Settings.

#include "RMCCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Movement/RMCMovementComponent.h"

// Sets default values
ARMCCharacter::ARMCCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<URMCMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Camera tilt settings
	WallRunCameraTilt = 15.0f;
	WallRunCameraTiltSpeed = 5.0f;
	SlideCameraLowerOffset = 60.0f;
	SlideCameraSpeed = 10.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bRotateRootBoneWithController = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// Initialize debug properties
	bDebugModeEnabled = false;
	bEnhanceWallRunning = false;
	WallRunSpeedMultiplier = 1.2f;

	// Configure character movement
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (MovementComponent)
	{
		MovementComponent->bOrientRotationToMovement = true; // Character moves in the direction of input...
		MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
		MovementComponent->JumpZVelocity = 600.f;
		MovementComponent->AirControl = 0.2f;

		// Set default momentum-based movement values
		MovementComponent->WallRunSpeed = 800.0f;
		MovementComponent->WallRunGravityScale = 0.25f;
		MovementComponent->WallRunJumpOffForce = 500.0f;
		MovementComponent->MinWallRunHeight = 50.0f;
		MovementComponent->MaxWallRunTime = 2.5f;
		MovementComponent->WallRunControlMultiplier = 0.5f;

		MovementComponent->SlideSpeed = 1200.0f;
		MovementComponent->SlideFriction = 0.2f;
		MovementComponent->SlideMinDuration = 0.5f;
		MovementComponent->SlideMaxDuration = 2.0f;
		MovementComponent->SlideMinSpeed = 200.0f;

		MovementComponent->DashDistance = 500.0f;
		MovementComponent->DashDuration = 0.2f;
		MovementComponent->DashCooldown = 1.0f;
		MovementComponent->DashGroundSpeedBoost = 500.0f;
		MovementComponent->DashAirSpeedBoost = 300.0f;

		MovementComponent->DoubleJumpZVelocity = 600.0f;

		MovementComponent->MomentumRetentionRate = 0.9f;
		MovementComponent->MaxMomentum = 100.0f;
		MovementComponent->MomentumDecayRate = 5.0f;
		MovementComponent->MomentumBuildRate = 10.0f;
	}

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Initialize movement input values
	ForwardInputValue = 0.0f;
	RightInputValue = 0.0f;

	// Initialize animation properties
	bIsWallRunningLeft = false;
	bIsWallRunningRight = false;
}

// Called when the game starts or when spawned
void ARMCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Store default camera values
	DefaultCameraBoomLength = CameraBoom->TargetArmLength;
	DefaultCameraLocation = FollowCamera->GetRelativeLocation();
	DefaultCameraRotation = FollowCamera->GetRelativeRotation();

	// Bind to movement component events
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (MovementComponent)
	{
		MovementComponent->OnWallRunBegin.AddDynamic(this, &ARMCCharacter::HandleWallRunBegin);
		MovementComponent->OnWallRunEnd.AddDynamic(this, &ARMCCharacter::HandleWallRunEnd);
		MovementComponent->OnSlideBegin.AddDynamic(this, &ARMCCharacter::HandleSlideBegin);
		MovementComponent->OnSlideEnd.AddDynamic(this, &ARMCCharacter::HandleSlideEnd);
		MovementComponent->OnDashBegin.AddDynamic(this, &ARMCCharacter::HandleDashBegin);
		MovementComponent->OnDashEnd.AddDynamic(this, &ARMCCharacter::HandleDashEnd);
		MovementComponent->OnMomentumChanged.AddDynamic(this, &ARMCCharacter::HandleMomentumChanged);
	}

	// Set up wall run check timer
	GetWorldTimerManager().SetTimer(TimerHandle_CheckWallRun, this, &ARMCCharacter::TryWallRun, 0.1f, true);
}

// Called every frame
void ARMCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update camera based on movement state
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (MovementComponent)
	{
		if (MovementComponent->bIsWallRunning)
		{
			UpdateCameraDuringWallRun(DeltaTime);
		}
		else if (MovementComponent->bIsSliding)
		{
			UpdateCameraDuringSlide(DeltaTime);
		}
		else
		{
			ResetCameraToDefault(DeltaTime);
		}
	}
	
	// Draw debug helpers if debug mode is enabled
    if (bDebugModeEnabled)
    {
        DrawDebugHelpers(0.0f);
    }
}

// Called to bind functionality to input
void ARMCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &ARMCCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARMCCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ARMCCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ARMCCharacter::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARMCCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARMCCharacter::LookUpAtRate);

	// Set up action bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ARMCCharacter::OnJumpActionPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ARMCCharacter::OnJumpActionReleased);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ARMCCharacter::OnDashActionPressed);
	PlayerInputComponent->BindAction("Slide", IE_Pressed, this, &ARMCCharacter::OnSlideActionPressed);
	PlayerInputComponent->BindAction("Slide", IE_Released, this, &ARMCCharacter::OnSlideActionReleased);
}

URMCMovementComponent* ARMCCharacter::GetRMCMovementComponent() const
{
	return Cast<URMCMovementComponent>(GetCharacterMovement());
}

//////////////////////////////////////////////////////////////////////////
// Input handlers

void ARMCCharacter::MoveForward(float Value)
{
	ForwardInputValue = Value;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARMCCharacter::MoveRight(float Value)
{
	RightInputValue = Value;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// Get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ARMCCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ARMCCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ARMCCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARMCCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//////////////////////////////////////////////////////////////////////////
// Momentum-based movement actions

void ARMCCharacter::OnJumpActionPressed()
{
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (MovementComponent)
	{
		// If wall running, perform wall jump
		if (MovementComponent->bIsWallRunning)
		{
			MovementComponent->WallRunJump();
		}
		// If in air and can double jump, perform double jump
		else if (MovementComponent->IsFalling() && MovementComponent->CanDoubleJump())
		{
			MovementComponent->PerformDoubleJump();
			// Call blueprint native event
			OnDoubleJump();
		}
		// Otherwise, normal jump
		else
		{
			Jump();
		}
	}
	else
	{
		Jump();
	}
}

void ARMCCharacter::OnJumpActionReleased()
{
	StopJumping();
}

void ARMCCharacter::OnDashActionPressed()
{
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (MovementComponent && MovementComponent->CanDash())
	{
		MovementComponent->PerformDash();
	}
}

void ARMCCharacter::OnSlideActionPressed()
{
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (MovementComponent && MovementComponent->CanSlide())
	{
		MovementComponent->StartSlide();
	}
}

void ARMCCharacter::OnSlideActionReleased()
{
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (MovementComponent && MovementComponent->bIsSliding)
	{
		MovementComponent->EndSlide();
	}
}

void ARMCCharacter::TryWallRun()
{
	URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
	if (!MovementComponent)
	{
		return;
	}
	
	// If already wall running, no need to check
	if (MovementComponent->bIsWallRunning)
	{
		return;
	}
	
	// Check if we're in a state where wall running is possible
	if (MovementComponent->IsFalling() && !MovementComponent->bIsSliding && !MovementComponent->bIsDashing)
	{
		// Check velocity - must be moving at a decent speed
		float Speed = MovementComponent->Velocity.Size2D();
		if (Speed > 200.0f)
		{
			// Use CanWallRun instead of directly calling FindWallRunSurface
			if (MovementComponent->CanWallRun())
			{
				// Start wall running
				MovementComponent->StartWallRun();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Movement component event handlers

void ARMCCharacter::HandleWallRunBegin(const FVector& WallNormal)
{
	// Determine if wall running on left or right side
	FVector Right = GetActorRightVector();
	float DotProduct = FVector::DotProduct(Right, WallNormal);

	bIsWallRunningLeft = DotProduct > 0;
	bIsWallRunningRight = DotProduct < 0;

	// Call blueprint native event
	OnWallRunBegin(WallNormal);
}

void ARMCCharacter::OnWallRunBegin_Implementation(const FVector& WallNormal)
{
	// Default implementation - can be overridden in Blueprints
}

void ARMCCharacter::HandleWallRunEnd()
{
	bIsWallRunningLeft = false;
	bIsWallRunningRight = false;

	// Call blueprint native event
	OnWallRunEnd();
}

void ARMCCharacter::OnWallRunEnd_Implementation()
{
	// Default implementation - can be overridden in Blueprints
}

void ARMCCharacter::HandleSlideBegin()
{
	// Call blueprint native event
	OnSlideBegin();
}

void ARMCCharacter::OnSlideBegin_Implementation()
{
	// Default implementation - can be overridden in Blueprints
}

void ARMCCharacter::HandleSlideEnd()
{
	// Call blueprint native event
	OnSlideEnd();
}

void ARMCCharacter::OnSlideEnd_Implementation()
{
	// Default implementation - can be overridden in Blueprints
}

void ARMCCharacter::HandleDashBegin(const FVector& DashDirection)
{
	// Call blueprint native event
	OnDashBegin(DashDirection);
}

void ARMCCharacter::OnDashBegin_Implementation(const FVector& DashDirection)
{
	// Default implementation - can be overridden in Blueprints
}

void ARMCCharacter::HandleDashEnd()
{
	// Call blueprint native event
	OnDashEnd();
}

void ARMCCharacter::OnDashEnd_Implementation()
{
	// Default implementation - can be overridden in Blueprints
}

void ARMCCharacter::HandleMomentumChanged(float NewMomentum)
{
	// Call blueprint native event
	OnMomentumChanged(NewMomentum);
}

void ARMCCharacter::OnMomentumChanged_Implementation(float NewMomentum)
{
	// Default implementation - can be overridden in Blueprints
}

void ARMCCharacter::OnDoubleJump_Implementation()
{
	// Default implementation - can be overridden in Blueprints
}

//////////////////////////////////////////////////////////////////////////
// Debug Helper Functions

void ARMCCharacter::ToggleDebugMode()
{
    bDebugModeEnabled = !bDebugModeEnabled;
    
    if (bDebugModeEnabled)
    {
        UE_LOG(LogTemp, Display, TEXT("Debug mode enabled for %s"), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Display, TEXT("Debug mode disabled for %s"), *GetName());
    }
}

void ARMCCharacter::DebugWallRunning()
{
    URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
    if (MovementComponent)
    {
        MovementComponent->DebugWallRunning(true, true, 2.0f);
    }
}

void ARMCCharacter::EnhanceWallRunning(float SpeedMultiplier)
{
    URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
    if (MovementComponent && MovementComponent->bIsWallRunning)
    {
        MovementComponent->ForceWallRunSpeed(SpeedMultiplier);
    }
}

FString ARMCCharacter::GetDebugInfo() const
{
    FString DebugInfo;
    
    URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
    if (MovementComponent)
    {
        DebugInfo += MovementComponent->GetMovementStateDebugString();
        DebugInfo += FString::Printf(TEXT("\nVelocity: %.1f"), MovementComponent->Velocity.Size());
        DebugInfo += FString::Printf(TEXT("\nMomentum: %.1f / %.1f"), 
            MovementComponent->GetCurrentMomentum(), MovementComponent->MaxMomentum);
        
        if (MovementComponent->bIsWallRunning)
        {
            DebugInfo += FString::Printf(TEXT("\nWall Run Time: %.1f / %.1f"), 
                MovementComponent->WallRunTimeRemaining, MovementComponent->MaxWallRunTime);
        }
    }
    
    DebugInfo += FString::Printf(TEXT("\nForward Input: %.2f"), ForwardInputValue);
    DebugInfo += FString::Printf(TEXT("\nRight Input: %.2f"), RightInputValue);
    
    return DebugInfo;
}

void ARMCCharacter::DrawDebugHelpers(float Duration)
{
    if (!bDebugModeEnabled)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Draw character forward direction
    DrawDebugLine(
        World,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * 100.0f,
        FColor::Yellow,
        false,
        Duration,
        0,
        2.0f
    );
    
    // Draw character right direction
    DrawDebugLine(
        World,
        GetActorLocation(),
        GetActorLocation() + GetActorRightVector() * 100.0f,
        FColor::Purple,
        false,
        Duration,
        0,
        2.0f
    );
    
    // Draw debug info text
    DrawDebugString(
        World,
        GetActorLocation() + FVector(0, 0, 150),
        GetDebugInfo(),
        nullptr,
        FColor::White,
        Duration
    );
    
    // If wall running, enhance it if enabled
    URMCMovementComponent* MovementComponent = GetRMCMovementComponent();
    if (MovementComponent && MovementComponent->bIsWallRunning && bEnhanceWallRunning)
    {
        EnhanceWallRunning(WallRunSpeedMultiplier);
    }
    
    // Call movement component debug drawing
    if (MovementComponent)
    {
        MovementComponent->DrawWallRunDebugHelpers(Duration);
    }
}

//////////////////////////////////////////////////////////////////////////
// Camera control functions

void ARMCCharacter::UpdateCameraDuringWallRun(float DeltaTime)
{
	if (bIsWallRunningLeft || bIsWallRunningRight)
	{
		// Calculate target roll based on which side we're wall running on
		float TargetRoll = bIsWallRunningLeft ? -WallRunCameraTilt : WallRunCameraTilt;
		
		// Smoothly interpolate to the target roll
		FRotator CurrentRotation = FollowCamera->GetRelativeRotation();
		FRotator TargetRotation = FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, TargetRoll);
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, WallRunCameraTiltSpeed);
		
		FollowCamera->SetRelativeRotation(NewRotation);
	}
}

void ARMCCharacter::UpdateCameraDuringSlide(float DeltaTime)
{
	// Lower the camera during slide
	FVector CurrentLocation = FollowCamera->GetRelativeLocation();
	FVector TargetLocation = FVector(CurrentLocation.X, CurrentLocation.Y, -SlideCameraLowerOffset);
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, SlideCameraSpeed);
	
	FollowCamera->SetRelativeLocation(NewLocation);

	// Reset any roll from wall running
	FRotator CurrentRotation = FollowCamera->GetRelativeRotation();
	FRotator TargetRotation = FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, 0.0f);
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, WallRunCameraTiltSpeed);
	
	FollowCamera->SetRelativeRotation(NewRotation);
}

void ARMCCharacter::ResetCameraToDefault(float DeltaTime)
{
	// Reset camera location
	FVector CurrentLocation = FollowCamera->GetRelativeLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, DefaultCameraLocation, DeltaTime, SlideCameraSpeed);
	FollowCamera->SetRelativeLocation(NewLocation);

	// Reset camera rotation
	FRotator CurrentRotation = FollowCamera->GetRelativeRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DefaultCameraRotation, DeltaTime, WallRunCameraTiltSpeed);
	FollowCamera->SetRelativeRotation(NewRotation);
}