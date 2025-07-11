// Fill out your copyright notice in the Description page of Project Settings.

#include "RMCMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "../../RMCCharacter.h"

// Custom movement mode enum values
static const uint8 CMOVE_WallRunning = 0;
static const uint8 CMOVE_Sliding = 1;
static const uint8 CMOVE_Dashing = 2;

URMCMovementComponent::URMCMovementComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Initialize the default physics profile
    InitializeDefaultPhysicsProfile();
    
    // Set default values for movement properties from default profile
    WallRunSpeed = DefaultPhysicsProfile.WallRunSpeed;
    WallRunGravityScale = DefaultPhysicsProfile.WallRunGravityScale;
    WallRunJumpOffForce = DefaultPhysicsProfile.WallRunJumpOffForce;
    MinWallRunHeight = DefaultPhysicsProfile.MinWallRunHeight;
    MaxWallRunTime = DefaultPhysicsProfile.MaxWallRunTime;
    WallRunControlMultiplier = DefaultPhysicsProfile.WallRunControlMultiplier;
    WallAttractionForce = DefaultPhysicsProfile.WallAttractionForce;
    MaxWallRunSurfaceAngle = DefaultPhysicsProfile.MaxWallRunSurfaceAngle;

    SlideSpeed = DefaultPhysicsProfile.SlideSpeed;
    SlideFriction = DefaultPhysicsProfile.SlideFriction;
    SlideMinDuration = DefaultPhysicsProfile.SlideMinDuration;
    SlideMaxDuration = DefaultPhysicsProfile.SlideMaxDuration;
    SlideMinSpeed = DefaultPhysicsProfile.SlideMinSpeed;
    SlideDownhillAccelerationMultiplier = DefaultPhysicsProfile.SlideDownhillAccelerationMultiplier;
    SlideCapsuleHeightScale = DefaultPhysicsProfile.SlideCapsuleHeightScale;

    DashDistance = DefaultPhysicsProfile.DashDistance;
    DashDuration = DefaultPhysicsProfile.DashDuration;
    DashCooldown = DefaultPhysicsProfile.DashCooldown;
    DashGroundSpeedBoost = DefaultPhysicsProfile.DashGroundSpeedBoost;
    DashAirSpeedBoost = DefaultPhysicsProfile.DashAirSpeedBoost;

    DoubleJumpZVelocity = DefaultPhysicsProfile.DoubleJumpZVelocity;

    MomentumRetentionRate = DefaultPhysicsProfile.MomentumRetentionRate;
    MaxMomentum = DefaultPhysicsProfile.MaxMomentum;
    MomentumDecayRate = DefaultPhysicsProfile.MomentumDecayRate;
    MomentumBuildRate = DefaultPhysicsProfile.MomentumBuildRate;
    MomentumSpeedMultiplier = DefaultPhysicsProfile.MomentumSpeedMultiplier;
    MomentumAccelerationMultiplier = DefaultPhysicsProfile.MomentumAccelerationMultiplier;

    // Add default profile to profiles array
    DefaultPhysicsProfile.ProfileName = TEXT("Default");
    PhysicsProfiles.Add(DefaultPhysicsProfile);
    CurrentProfileName = DefaultPhysicsProfile.ProfileName;

    // Initialize movement states
    bIsWallRunning = false;
    bIsSliding = false;
    bIsDashing = false;
    bHasDoubleJumped = false;
    CurrentMomentum = 0.0f;
    DashCooldownRemaining = 0.0f;
    WallRunTimeRemaining = 0.0f;
    SlideTimeRemaining = 0.0f;
    CurrentWallNormal = FVector::ZeroVector;
    DashDirection = FVector::ZeroVector;
    
    // Initialize speed cap properties
    GlobalSpeedCap = 3000.0f;
    SpeedCapDamping = 0.8f;
    bApplySpeedCapToZVelocity = false;

    // Set component to tick
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void URMCMovementComponent::InitializeDefaultPhysicsProfile()
{
    // Initialize with default values
    DefaultPhysicsProfile.ProfileName = TEXT("Default");
    
    // Wall Running
    DefaultPhysicsProfile.WallRunSpeed = 800.0f;
    DefaultPhysicsProfile.WallRunGravityScale = 0.25f;
    DefaultPhysicsProfile.WallRunJumpOffForce = 500.0f;
    DefaultPhysicsProfile.MinWallRunHeight = 50.0f;
    DefaultPhysicsProfile.MaxWallRunTime = 2.5f;
    DefaultPhysicsProfile.WallRunControlMultiplier = 0.5f;
    DefaultPhysicsProfile.WallAttractionForce = 200.0f;
    DefaultPhysicsProfile.MaxWallRunSurfaceAngle = 11.0f;
    
    // Sliding
    DefaultPhysicsProfile.SlideSpeed = 1200.0f;
    DefaultPhysicsProfile.SlideFriction = 0.2f;
    DefaultPhysicsProfile.SlideMinDuration = 0.5f;
    DefaultPhysicsProfile.SlideMaxDuration = 2.0f;
    DefaultPhysicsProfile.SlideMinSpeed = 200.0f;
    DefaultPhysicsProfile.SlideDownhillAccelerationMultiplier = 2.0f;
    DefaultPhysicsProfile.SlideCapsuleHeightScale = 0.5f;
    
    // Dashing
    DefaultPhysicsProfile.DashDistance = 500.0f;
    DefaultPhysicsProfile.DashDuration = 0.2f;
    DefaultPhysicsProfile.DashCooldown = 1.0f;
    DefaultPhysicsProfile.DashGroundSpeedBoost = 500.0f;
    DefaultPhysicsProfile.DashAirSpeedBoost = 300.0f;
    
    // Double Jump
    DefaultPhysicsProfile.DoubleJumpZVelocity = 600.0f;
    
    // Momentum
    DefaultPhysicsProfile.MomentumRetentionRate = 0.9f;
    DefaultPhysicsProfile.MaxMomentum = 100.0f;
    DefaultPhysicsProfile.MomentumDecayRate = 5.0f;
    DefaultPhysicsProfile.MomentumBuildRate = 10.0f;
    DefaultPhysicsProfile.MomentumSpeedMultiplier = 0.5f;
    DefaultPhysicsProfile.MomentumAccelerationMultiplier = 0.3f;
    
    // Speed Cap
    DefaultPhysicsProfile.GlobalSpeedCap = 3000.0f;
    DefaultPhysicsProfile.SpeedCapDamping = 0.8f;
    DefaultPhysicsProfile.bApplySpeedCapToZVelocity = false;
}

void URMCMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize momentum
    CurrentMomentum = MaxMomentum * 0.5f;
    OnMomentumChanged.Broadcast(CurrentMomentum);
    
    // Apply the current profile (in case it was changed in editor)
    if (CurrentProfileName != NAME_None)
    {
        SetMovementPhysicsProfile(CurrentProfileName);
    }
}

// Physics Profile Management
bool URMCMovementComponent::SetMovementPhysicsProfile(FName ProfileName)
{
    // Find the profile with the given name
    for (const FMovementPhysicsProfile& Profile : PhysicsProfiles)
    {
        if (Profile.ProfileName == ProfileName)
        {
            // Apply the profile settings
            // Wall Running
            WallRunSpeed = Profile.WallRunSpeed;
            WallRunGravityScale = Profile.WallRunGravityScale;
            WallRunJumpOffForce = Profile.WallRunJumpOffForce;
            MinWallRunHeight = Profile.MinWallRunHeight;
            MaxWallRunTime = Profile.MaxWallRunTime;
            WallRunControlMultiplier = Profile.WallRunControlMultiplier;
            WallAttractionForce = Profile.WallAttractionForce;
            MaxWallRunSurfaceAngle = Profile.MaxWallRunSurfaceAngle;
            
            // Sliding
            SlideSpeed = Profile.SlideSpeed;
            SlideFriction = Profile.SlideFriction;
            SlideMinDuration = Profile.SlideMinDuration;
            SlideMaxDuration = Profile.SlideMaxDuration;
            SlideMinSpeed = Profile.SlideMinSpeed;
            SlideDownhillAccelerationMultiplier = Profile.SlideDownhillAccelerationMultiplier;
            SlideCapsuleHeightScale = Profile.SlideCapsuleHeightScale;
            
            // Dashing
            DashDistance = Profile.DashDistance;
            DashDuration = Profile.DashDuration;
            DashCooldown = Profile.DashCooldown;
            DashGroundSpeedBoost = Profile.DashGroundSpeedBoost;
            DashAirSpeedBoost = Profile.DashAirSpeedBoost;
            
            // Double Jump
            DoubleJumpZVelocity = Profile.DoubleJumpZVelocity;
            
            // Momentum
            MomentumRetentionRate = Profile.MomentumRetentionRate;
            MaxMomentum = Profile.MaxMomentum;
            MomentumDecayRate = Profile.MomentumDecayRate;
            MomentumBuildRate = Profile.MomentumBuildRate;
            MomentumSpeedMultiplier = Profile.MomentumSpeedMultiplier;
            MomentumAccelerationMultiplier = Profile.MomentumAccelerationMultiplier;
            
            // Speed Cap
            GlobalSpeedCap = Profile.GlobalSpeedCap;
            SpeedCapDamping = Profile.SpeedCapDamping;
            bApplySpeedCapToZVelocity = Profile.bApplySpeedCapToZVelocity;
            
            // Update current profile name
            CurrentProfileName = ProfileName;
            
            // Broadcast event
            OnPhysicsProfileChanged.Broadcast(ProfileName);
            OnPhysicsProfileChanged_BP(ProfileName);
            
            return true;
        }
    }
    
    // Profile not found
    return false;
}

void URMCMovementComponent::ResetMovementPhysicsToDefaults()
{
    // Apply the default profile
    SetMovementPhysicsProfile(TEXT("Default"));
}

void URMCMovementComponent::SaveCurrentPhysicsAsProfile(FName ProfileName)
{
    if (ProfileName == NAME_None)
    {
        return;
    }
    
    // Check if profile already exists
    for (int32 i = 0; i < PhysicsProfiles.Num(); i++)
    {
        if (PhysicsProfiles[i].ProfileName == ProfileName)
        {
            // Update existing profile
            FMovementPhysicsProfile& Profile = PhysicsProfiles[i];
            
            // Wall Running
            Profile.WallRunSpeed = WallRunSpeed;
            Profile.WallRunGravityScale = WallRunGravityScale;
            Profile.WallRunJumpOffForce = WallRunJumpOffForce;
            Profile.MinWallRunHeight = MinWallRunHeight;
            Profile.MaxWallRunTime = MaxWallRunTime;
            Profile.WallRunControlMultiplier = WallRunControlMultiplier;
            Profile.WallAttractionForce = WallAttractionForce;
            Profile.MaxWallRunSurfaceAngle = MaxWallRunSurfaceAngle;
            
            // Sliding
            Profile.SlideSpeed = SlideSpeed;
            Profile.SlideFriction = SlideFriction;
            Profile.SlideMinDuration = SlideMinDuration;
            Profile.SlideMaxDuration = SlideMaxDuration;
            Profile.SlideMinSpeed = SlideMinSpeed;
            Profile.SlideDownhillAccelerationMultiplier = SlideDownhillAccelerationMultiplier;
            Profile.SlideCapsuleHeightScale = SlideCapsuleHeightScale;
            
            // Dashing
            Profile.DashDistance = DashDistance;
            Profile.DashDuration = DashDuration;
            Profile.DashCooldown = DashCooldown;
            Profile.DashGroundSpeedBoost = DashGroundSpeedBoost;
            Profile.DashAirSpeedBoost = DashAirSpeedBoost;
            
            // Double Jump
            Profile.DoubleJumpZVelocity = DoubleJumpZVelocity;
            
            // Momentum
            Profile.MomentumRetentionRate = MomentumRetentionRate;
            Profile.MaxMomentum = MaxMomentum;
            Profile.MomentumDecayRate = MomentumDecayRate;
            Profile.MomentumBuildRate = MomentumBuildRate;
            Profile.MomentumSpeedMultiplier = MomentumSpeedMultiplier;
            Profile.MomentumAccelerationMultiplier = MomentumAccelerationMultiplier;
            
            // Speed Cap
            Profile.GlobalSpeedCap = GlobalSpeedCap;
            Profile.SpeedCapDamping = SpeedCapDamping;
            Profile.bApplySpeedCapToZVelocity = bApplySpeedCapToZVelocity;
            
            CurrentProfileName = ProfileName;
            return;
        }
    }
    
    // Create new profile
    FMovementPhysicsProfile NewProfile;
    NewProfile.ProfileName = ProfileName;
    
    // Wall Running
    NewProfile.WallRunSpeed = WallRunSpeed;
    NewProfile.WallRunGravityScale = WallRunGravityScale;
    NewProfile.WallRunJumpOffForce = WallRunJumpOffForce;
    NewProfile.MinWallRunHeight = MinWallRunHeight;
    NewProfile.MaxWallRunTime = MaxWallRunTime;
    NewProfile.WallRunControlMultiplier = WallRunControlMultiplier;
    NewProfile.WallAttractionForce = WallAttractionForce;
    NewProfile.MaxWallRunSurfaceAngle = MaxWallRunSurfaceAngle;
    
    // Sliding
    NewProfile.SlideSpeed = SlideSpeed;
    NewProfile.SlideFriction = SlideFriction;
    NewProfile.SlideMinDuration = SlideMinDuration;
    NewProfile.SlideMaxDuration = SlideMaxDuration;
    NewProfile.SlideMinSpeed = SlideMinSpeed;
    NewProfile.SlideDownhillAccelerationMultiplier = SlideDownhillAccelerationMultiplier;
    NewProfile.SlideCapsuleHeightScale = SlideCapsuleHeightScale;
    
    // Dashing
    NewProfile.DashDistance = DashDistance;
    NewProfile.DashDuration = DashDuration;
    NewProfile.DashCooldown = DashCooldown;
    NewProfile.DashGroundSpeedBoost = DashGroundSpeedBoost;
    NewProfile.DashAirSpeedBoost = DashAirSpeedBoost;
    
    // Double Jump
    NewProfile.DoubleJumpZVelocity = DoubleJumpZVelocity;
    
    // Momentum
    NewProfile.MomentumRetentionRate = MomentumRetentionRate;
    NewProfile.MaxMomentum = MaxMomentum;
    NewProfile.MomentumDecayRate = MomentumDecayRate;
    NewProfile.MomentumBuildRate = MomentumBuildRate;
    NewProfile.MomentumSpeedMultiplier = MomentumSpeedMultiplier;
    NewProfile.MomentumAccelerationMultiplier = MomentumAccelerationMultiplier;
    
    // Speed Cap
    NewProfile.GlobalSpeedCap = GlobalSpeedCap;
    NewProfile.SpeedCapDamping = SpeedCapDamping;
    NewProfile.bApplySpeedCapToZVelocity = bApplySpeedCapToZVelocity;
    
    // Add to profiles array
    PhysicsProfiles.Add(NewProfile);
    CurrentProfileName = ProfileName;
}

TArray<FName> URMCMovementComponent::GetAvailablePhysicsProfileNames() const
{
    TArray<FName> ProfileNames;
    for (const FMovementPhysicsProfile& Profile : PhysicsProfiles)
    {
        ProfileNames.Add(Profile.ProfileName);
    }
    return ProfileNames;
}

// Specific Physics Setting Functions
void URMCMovementComponent::SetWallRunningPhysics(float Speed, float WallRunGravity, float JumpForce, float ControlMultiplier)
{
    WallRunSpeed = Speed;
    WallRunGravityScale = WallRunGravity;
    WallRunJumpOffForce = JumpForce;
    WallRunControlMultiplier = ControlMultiplier;
}

void URMCMovementComponent::SetSlidingPhysics(float Speed, float Friction, float DownhillAcceleration, float CapsuleScale)
{
    SlideSpeed = Speed;
    SlideFriction = Friction;
    SlideDownhillAccelerationMultiplier = DownhillAcceleration;
    SlideCapsuleHeightScale = CapsuleScale;
}

void URMCMovementComponent::SetDashingPhysics(float Distance, float Duration, float Cooldown, float GroundBoost, float AirBoost)
{
    DashDistance = Distance;
    DashDuration = Duration;
    DashCooldown = Cooldown;
    DashGroundSpeedBoost = GroundBoost;
    DashAirSpeedBoost = AirBoost;
}

void URMCMovementComponent::SetMomentumPhysics(float MaxValue, float BuildRate, float DecayRate, float SpeedMultiplier, float AccelMultiplier)
{
    MaxMomentum = MaxValue;
    MomentumBuildRate = BuildRate;
    MomentumDecayRate = DecayRate;
    MomentumSpeedMultiplier = SpeedMultiplier;
    MomentumAccelerationMultiplier = AccelMultiplier;
}

void URMCMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // Apply speed cap
    if (GlobalSpeedCap > 0.0f)
    {
        // Check if we need to apply the cap
        float CurrentSpeed;
        if (bApplySpeedCapToZVelocity)
        {
            // Apply cap to full 3D velocity
            CurrentSpeed = Velocity.Size();
        }
        else
        {
            // Apply cap only to horizontal velocity
            CurrentSpeed = Velocity.Size2D();
        }
        
        if (CurrentSpeed > GlobalSpeedCap)
        {
            // Calculate new speed with damping
            float NewSpeed = FMath::Lerp(CurrentSpeed, GlobalSpeedCap, 1.0f - SpeedCapDamping);
            
            if (bApplySpeedCapToZVelocity)
            {
                // Scale entire velocity
                Velocity = Velocity.GetSafeNormal() * NewSpeed;
            }
            else
            {
                // Scale only horizontal velocity
                FVector HorizontalVel = FVector(Velocity.X, Velocity.Y, 0.0f);
                FVector NewHorizontalVel = HorizontalVel.GetSafeNormal() * NewSpeed;
                Velocity.X = NewHorizontalVel.X;
                Velocity.Y = NewHorizontalVel.Y;
                // Z velocity is unchanged
            }
            
            // Debug output
            ACharacter* Character = Cast<ACharacter>(GetOwner());
            if (GEngine && Character && Character->IsA<ARMCCharacter>() && 
                Cast<ARMCCharacter>(Character)->bDebugModeEnabled)
            {
                GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, 
                    FString::Printf(TEXT("Speed Capped: %.1f → %.1f"), CurrentSpeed, NewSpeed));
            }
        }
    }
    
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update momentum based on current movement
    UpdateMomentum(DeltaTime);

    // Update cooldowns and timers
    UpdateDashCooldown(DeltaTime);
    UpdateWallRunTime(DeltaTime);
    UpdateSlideTime(DeltaTime);

    // Check if we should end wall running due to conditions
    if (bIsWallRunning)
    {
        FVector WallNormal;
        if (!FindWallRunSurface(WallNormal) || Velocity.SizeSquared() < 100.0f)
        {
            EndWallRun();
        }
    }

    // Check if we should end sliding due to conditions
    if (bIsSliding)
    {
        if (Velocity.SizeSquared() < FMath::Square(SlideMinSpeed) || !IsMovingOnGround())
        {
            EndSlide();
        }
    }
}

void URMCMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

    // Reset double jump when landing
    if (IsMovingOnGround() && PreviousMovementMode == MOVE_Falling)
    {
        bHasDoubleJumped = false;
    }

    // End wall running if we change to a non-wall running mode
    if (bIsWallRunning && (MovementMode != MOVE_Custom || CustomMovementMode != CMOVE_WallRunning))
    {
        EndWallRun();
    }

    // End sliding if we change to a non-sliding mode
    if (bIsSliding && (MovementMode != MOVE_Custom || CustomMovementMode != CMOVE_Sliding))
    {
        EndSlide();
    }

    // End dashing if we change to a non-dashing mode
    if (bIsDashing && (MovementMode != MOVE_Custom || CustomMovementMode != CMOVE_Dashing))
    {
        bIsDashing = false;
        OnDashEnd.Broadcast();
        OnDashEnd_BP();
    }
}

void URMCMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
    Super::PhysWalking(deltaTime, Iterations);

    // Apply momentum-based effects to walking
    if (CurrentMomentum > 0)
    {
        // Increase max speed based on momentum
        float MomentumFactor = FMath::Clamp(CurrentMomentum / MaxMomentum, 0.0f, 1.0f);
        MaxWalkSpeed = MaxWalkSpeed * (1.0f + MomentumFactor * 0.5f);
    }
}

void URMCMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    switch (CustomMovementMode)
    {
    case CMOVE_WallRunning:
        // Apply wall running physics
        ApplyWallRunForces(deltaTime, CurrentWallNormal);
        break;

    case CMOVE_Sliding:
        // Apply sliding physics
        ApplySlideForces(deltaTime);
        break;

    case CMOVE_Dashing:
        // Apply dashing physics
        ApplyDashForces(deltaTime);
        break;

    default:
        Super::PhysCustom(deltaTime, Iterations);
        break;
    }
}

bool URMCMovementComponent::DoJump(bool bReplayingMoves)
{
    // If wall running, perform wall jump instead
    if (bIsWallRunning)
    {
        WallRunJump();
        return true;
    }
    
    // If in air and can double jump, perform double jump
    if (IsFalling() && CanDoubleJump())
    {
        PerformDoubleJump();
        return true;
    }

    // Otherwise, perform normal jump
    return Super::DoJump(bReplayingMoves);
}

float URMCMovementComponent::GetMaxSpeed() const
{
    // Adjust max speed based on movement state
    if (bIsWallRunning)
    {
        return WallRunSpeed;
    }
    else if (bIsSliding)
    {
        return SlideSpeed;
    }
    else if (bIsDashing)
    {
        return FMath::Max(Super::GetMaxSpeed(), Velocity.Size());
    }
    
    // Apply momentum boost to normal max speed using the configurable multiplier
    float MomentumFactor = FMath::Clamp(CurrentMomentum / MaxMomentum, 0.0f, 1.0f);
    return Super::GetMaxSpeed() * (1.0f + MomentumFactor * MomentumSpeedMultiplier);
}

float URMCMovementComponent::GetMaxAcceleration() const
{
    // Adjust acceleration based on movement state
    if (bIsWallRunning)
    {
        return Super::GetMaxAcceleration() * 1.5f;
    }
    else if (bIsSliding)
    {
        return Super::GetMaxAcceleration() * 0.5f;
    }
    else if (bIsDashing)
    {
        return Super::GetMaxAcceleration() * 2.0f;
    }
    
    // Apply momentum boost to normal acceleration using the configurable multiplier
    float MomentumFactor = FMath::Clamp(CurrentMomentum / MaxMomentum, 0.0f, 1.0f);
    return Super::GetMaxAcceleration() * (1.0f + MomentumFactor * MomentumAccelerationMultiplier);
}

//////////////////////////////////////////////////////////////////////////
// Wall Running Implementation

void URMCMovementComponent::StartWallRun()
{
    // Don't start if already wall running
    if (bIsWallRunning)
    {
        return;
    }

    // Find a valid wall to run on
    FVector WallNormal;
    if (!FindWallRunSurface(WallNormal))
    {
        return;
    }

    // Set wall running state
    bIsWallRunning = true;
    CurrentWallNormal = WallNormal;
    WallRunTimeRemaining = MaxWallRunTime;

    // Set custom movement mode
    SetMovementMode(MOVE_Custom, CMOVE_WallRunning);

    // Calculate wall run direction
    FVector WallRunDirection = FVector::CrossProduct(WallNormal, FVector(0, 0, 1)).GetSafeNormal();
    
    // Determine the best direction to run based on current velocity and input
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    FVector InputVector = FVector::ZeroVector;
    if (Character && Character->InputComponent)
    {
        InputVector = ConsumeInputVector();
    }
    
    // Check if velocity is already along the wall
    float DotWithWallDir = FVector::DotProduct(Velocity.GetSafeNormal2D(), WallRunDirection);
    float DotWithNegWallDir = FVector::DotProduct(Velocity.GetSafeNormal2D(), -WallRunDirection);
    
    // If velocity is more aligned with the negative wall direction, reverse it
    if (DotWithNegWallDir > DotWithWallDir)
    {
        WallRunDirection = -WallRunDirection;
    }
    
    // If we have input, use it to influence the wall run direction
    if (!InputVector.IsNearlyZero())
    {
        // Project input onto the wall plane
        FVector InputAlongWall = FVector::VectorPlaneProject(InputVector, WallNormal).GetSafeNormal2D();
        
        // If input is more aligned with the negative wall direction, reverse it
        if (FVector::DotProduct(InputAlongWall, -WallRunDirection) > 
            FVector::DotProduct(InputAlongWall, WallRunDirection))
        {
            WallRunDirection = -WallRunDirection;
        }
    }
    
    // Set initial velocity along the wall
    float InitialSpeed = FMath::Max(Velocity.Size2D(), WallRunSpeed);
    Velocity = WallRunDirection * InitialSpeed;
    
    // Preserve some of the Z velocity to make transitions smoother
    if (Velocity.Z > 0)
    {
        Velocity.Z *= 0.5f;
    }
    else
    {
        Velocity.Z = 0;
    }

    // Add momentum
    AddMomentum(10.0f);

    // Broadcast events
    OnWallRunBegin.Broadcast(WallNormal);
    OnWallRunBegin_BP(WallNormal);
    
    // Debug output
    if (GEngine && Character && Character->IsA<ARMCCharacter>() && 
        Cast<ARMCCharacter>(Character)->bDebugModeEnabled)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
            FString::Printf(TEXT("Wall Run Started: Speed=%.1f"), Velocity.Size()));
    }
}

void URMCMovementComponent::EndWallRun()
{
    if (!bIsWallRunning)
    {
        return;
    }

    // Reset wall running state
    bIsWallRunning = false;
    CurrentWallNormal = FVector::ZeroVector;
    WallRunTimeRemaining = 0.0f;

    // Return to falling movement mode
    SetMovementMode(MOVE_Falling);

    // Broadcast events
    OnWallRunEnd.Broadcast();
    OnWallRunEnd_BP();
}

bool URMCMovementComponent::CanWallRun() const
{
    // Can't wall run if on ground
    if (IsMovingOnGround())
    {
        return false;
    }

    // Can't wall run if not moving fast enough
    if (Velocity.SizeSquared() < 10000.0f) // 100 units/sec
    {
        return false;
    }

    // Check if we have enough momentum
    if (!HasMinimumMomentumForAction(MaxMomentum * 0.2f))
    {
        return false;
    }

    // Check if there's a valid wall to run on
    FVector WallNormal;
    return FindWallRunSurface(WallNormal);
}

void URMCMovementComponent::WallRunJump()
{
    if (!bIsWallRunning)
    {
        return;
    }

    // Calculate jump direction (away from wall and upward)
    FVector JumpDirection = CurrentWallNormal + FVector(0, 0, 0.5f);
    JumpDirection.Normalize();

    // Apply jump force
    Velocity = JumpDirection * WallRunJumpOffForce;
    
    // Add upward velocity
    Velocity.Z = JumpZVelocity;

    // End wall running
    EndWallRun();

    // Add momentum
    AddMomentum(15.0f);
}

bool URMCMovementComponent::FindWallRunSurface(FVector& OutWallNormal) const
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return false;
    }

    // Get character dimensions
    UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
    const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    
    // Check if we're high enough off the ground
    FVector Start = Character->GetActorLocation();
    FVector End = Start - FVector(0, 0, MinWallRunHeight + CapsuleHalfHeight);
    
    FHitResult FloorHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    if (GetWorld()->LineTraceSingleByChannel(FloorHit, Start, End, ECC_Visibility, QueryParams))
    {
        // Too close to the ground
        return false;
    }
    
    // Get character velocity direction for better wall detection
    FVector VelocityDir = Velocity.GetSafeNormal2D();
    if (VelocityDir.IsNearlyZero())
    {
        // If not moving, use forward vector
        VelocityDir = Character->GetActorForwardVector();
    }
    
    // Directions to check - more comprehensive set of directions
    TArray<FVector> DirectionsToCheck;
    
    // Add velocity-based directions
    DirectionsToCheck.Add(VelocityDir);
    
    // Add perpendicular directions (for side approaches)
    FVector RightDir = FVector::CrossProduct(VelocityDir, FVector(0, 0, 1)).GetSafeNormal();
    DirectionsToCheck.Add(RightDir);
    DirectionsToCheck.Add(-RightDir);
    
    // Add diagonal directions
    DirectionsToCheck.Add((VelocityDir + RightDir).GetSafeNormal());
    DirectionsToCheck.Add((VelocityDir - RightDir).GetSafeNormal());
    
    // Add character-relative directions
    DirectionsToCheck.Add(Character->GetActorForwardVector());
    DirectionsToCheck.Add(Character->GetActorRightVector());
    DirectionsToCheck.Add(-Character->GetActorRightVector());
    
    // Calculate max Z component based on MaxWallRunSurfaceAngle
    // Convert degrees to radians and find the sine
    const float MaxZComponent = FMath::Sin(FMath::DegreesToRadians(MaxWallRunSurfaceAngle));
    
    // Trace distance slightly beyond capsule radius
    const float TraceDistance = CapsuleRadius + 20.0f;
    
    // Debug
    bool bDebugDraw = false;
    if (Character->IsA<ARMCCharacter>())
    {
        bDebugDraw = Cast<ARMCCharacter>(Character)->bDebugModeEnabled;
    }
    
    // Check each direction
    for (const FVector& Direction : DirectionsToCheck)
    {
        Start = Character->GetActorLocation();
        End = Start + Direction * TraceDistance;
        
        FHitResult WallHit;
        if (GetWorld()->LineTraceSingleByChannel(WallHit, Start, End, ECC_Visibility, QueryParams))
        {
            // Check if the surface is vertical enough to be a wall using the configurable angle
            if (FMath::Abs(WallHit.Normal.Z) < MaxZComponent)
            {
                // Debug visualization
                if (bDebugDraw)
                {
                    DrawDebugLine(
                        GetWorld(),
                        Start,
                        WallHit.Location,
                        FColor::Green,
                        false,
                        0.1f,
                        0,
                        2.0f
                    );
                    
                    DrawDebugLine(
                        GetWorld(),
                        WallHit.Location,
                        WallHit.Location + WallHit.Normal * 50.0f,
                        FColor::Red,
                        false,
                        0.1f,
                        0,
                        2.0f
                    );
                }
                
                OutWallNormal = WallHit.Normal;
                return true;
            }
        }
        else if (bDebugDraw)
        {
            // Debug visualization for failed traces
            DrawDebugLine(
                GetWorld(),
                Start,
                End,
                FColor::Red,
                false,
                0.1f,
                0,
                1.0f
            );
        }
    }
    
    return false;
}

void URMCMovementComponent::ApplyWallRunForces(float DeltaTime, const FVector& WallNormal)
{
    // Calculate wall run direction (along the wall)
    FVector WallRunDirection = FVector::CrossProduct(WallNormal, FVector(0, 0, 1)).GetSafeNormal();
    
    // Make sure we're running in the correct direction along the wall
    if (FVector::DotProduct(WallRunDirection, Velocity) < 0)
    {
        WallRunDirection = -WallRunDirection;
    }
    
    // Apply reduced gravity using the configurable gravity scale
    const float fGravityScale = WallRunGravityScale;
    const FVector Gravity = FVector(0, 0, GetGravityZ() * fGravityScale * DeltaTime);
    
    // Apply wall attraction force to keep character on the wall using the configurable force
    const FVector WallAttractionVector = -WallNormal * WallAttractionForce * DeltaTime;
    
    // Get input vector for direction control
    FVector InputVector = FVector::ZeroVector;
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character && Character->InputComponent)
    {
        InputVector = ConsumeInputVector();
    }
    
    // Calculate forward component of input (how much the player is pressing forward)
    float ForwardInput = FVector::DotProduct(InputVector, WallRunDirection);
    
    // Calculate final velocity using the configurable wall run speed
    // Use a higher base speed to ensure movement along the wall
    float SpeedMultiplier = 1.0f;
    
    // If player is pressing forward, give a speed boost
    if (ForwardInput > 0.1f)
    {
        SpeedMultiplier = 1.2f;
    }
    
    // Set base velocity along the wall
    Velocity = WallRunDirection * WallRunSpeed * SpeedMultiplier;
    
    // Apply gravity and wall attraction
    Velocity += Gravity;
    Velocity += WallAttractionVector;
    
    // Allow some control for the player using the configurable control multiplier
    if (!InputVector.IsNearlyZero())
    {
        // Apply input to velocity (only along the wall)
        FVector InputAlongWall = FVector::VectorPlaneProject(InputVector, WallNormal);
        Velocity += InputAlongWall * WallRunControlMultiplier * 800.0f * DeltaTime;
    }
    
    // Ensure minimum velocity along wall to prevent sticking
    float CurrentSpeed = Velocity.Size2D();
    if (CurrentSpeed < WallRunSpeed * 0.7f)
    {
        Velocity = WallRunDirection * WallRunSpeed * 0.7f;
        Velocity += Gravity;
        Velocity += WallAttractionVector;
    }
    
    // Debug output
    if (GEngine && Character && Character->IsA<ARMCCharacter>() && 
        Cast<ARMCCharacter>(Character)->bDebugModeEnabled)
    {
        GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, 
            FString::Printf(TEXT("Wall Run Speed: %.1f"), Velocity.Size()));
    }
}

void URMCMovementComponent::UpdateWallRunTime(float DeltaTime)
{
    if (bIsWallRunning)
    {
        WallRunTimeRemaining -= DeltaTime;
        
        // End wall run if time expires
        if (WallRunTimeRemaining <= 0)
        {
            EndWallRun();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Sliding Implementation

void URMCMovementComponent::StartSlide()
{
    // Don't start if already sliding
    if (bIsSliding)
    {
        return;
    }
    
    // Must be on ground and moving
    if (!IsMovingOnGround() || Velocity.SizeSquared() < FMath::Square(SlideMinSpeed))
    {
        return;
    }
    
    // Set sliding state
    bIsSliding = true;
    SlideTimeRemaining = SlideMaxDuration;
    
    // Set custom movement mode
    SetMovementMode(MOVE_Custom, CMOVE_Sliding);
    
    // Lower character capsule using the configurable height scale
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character)
    {
        UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
        Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() * SlideCapsuleHeightScale);
    }
    
    // Boost initial slide velocity
    FVector SlideDirection = Velocity.GetSafeNormal2D();
    Velocity = SlideDirection * SlideSpeed;
    
    // Add momentum
    AddMomentum(5.0f);
    
    // Broadcast events
    OnSlideBegin.Broadcast();
    OnSlideBegin_BP();
}

void URMCMovementComponent::EndSlide()
{
    if (!bIsSliding)
    {
        return;
    }
    
    // Reset sliding state
    bIsSliding = false;
    SlideTimeRemaining = 0.0f;
    
    // Return to walking movement mode if on ground
    if (IsMovingOnGround())
    {
        SetMovementMode(MOVE_Walking);
    }
    else
    {
        SetMovementMode(MOVE_Falling);
    }
    
    // Restore character capsule
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character)
    {
        UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
        // Calculate the scale factor to restore the original height
        // If we scaled by 0.5, we need to multiply by 1/0.5 = 2.0 to restore
        float RestoreScale = 1.0f / SlideCapsuleHeightScale;
        Capsule->SetCapsuleHalfHeight(Capsule->GetUnscaledCapsuleHalfHeight() * RestoreScale);
        
        // Adjust character position to prevent getting stuck in geometry
        FVector AdjustLocation = Character->GetActorLocation();
        // Calculate the height adjustment based on the capsule height change
        float HeightAdjustment = Capsule->GetScaledCapsuleHalfHeight() * (1.0f - SlideCapsuleHeightScale);
        AdjustLocation.Z += HeightAdjustment;
        Character->SetActorLocation(AdjustLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }
    
    // Broadcast events
    OnSlideEnd.Broadcast();
    OnSlideEnd_BP();
}

bool URMCMovementComponent::CanSlide() const
{
    // Must be on ground
    if (!IsMovingOnGround())
    {
        return false;
    }
    
    // Must be moving fast enough
    if (Velocity.SizeSquared() < FMath::Square(SlideMinSpeed))
    {
        return false;
    }
    
    // Check if we have enough momentum
    if (!HasMinimumMomentumForAction(MaxMomentum * 0.1f))
    {
        return false;
    }
    
    return true;
}

void URMCMovementComponent::ApplySlideForces(float DeltaTime)
{
    // Apply friction to slow down over time using the configurable friction value
    FVector SlideDirection = Velocity.GetSafeNormal2D();
    float CurrentSpeed = Velocity.Size2D();
    float NewSpeed = FMath::Max(CurrentSpeed - (SlideFriction * CurrentSpeed * DeltaTime), SlideMinSpeed);
    
    // Apply gravity component along slope
    FVector FloorNormal = CurrentFloor.HitResult.Normal;
    float FloorDot = FVector::DotProduct(FloorNormal, FVector(0, 0, 1));
    FVector fGravityDirection = FVector(0, 0, -1) - FloorNormal * FloorDot;
    
    // If on a slope, accelerate downhill using the configurable multiplier
    if (FloorDot < 0.9999f) // Not completely flat
    {
        FVector DownhillDirection = fGravityDirection.GetSafeNormal();
        float DownhillComponent = FVector::DotProduct(SlideDirection, DownhillDirection);
        
        // If sliding downhill, add acceleration
        if (DownhillComponent > 0)
        {
            // Use the configurable downhill acceleration multiplier
            NewSpeed += 500.0f * DownhillComponent * SlideDownhillAccelerationMultiplier * DeltaTime;
        }
    }
    
    // Allow some control for the player
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character)
    {
        FVector InputVector = ConsumeInputVector();
        SlideDirection = FMath::VInterpTo(
            SlideDirection,
            (SlideDirection + InputVector * 0.5f).GetSafeNormal(),
            DeltaTime,
            2.0f
        );
    }
    
    // Set new velocity, capped at the configurable slide speed
    Velocity = SlideDirection * FMath::Min(NewSpeed, SlideSpeed);
}

void URMCMovementComponent::UpdateSlideTime(float DeltaTime)
{
    if (bIsSliding)
    {
        SlideTimeRemaining -= DeltaTime;
        
        // End slide if minimum duration has passed and player isn't providing input
        if (SlideTimeRemaining <= (SlideMaxDuration - SlideMinDuration))
        {
            ACharacter* Character = Cast<ACharacter>(GetOwner());
            if (Character)
            {
                FVector InputVector = ConsumeInputVector();
                if (InputVector.SizeSquared() < 0.1f)
                {
                    EndSlide();
                }
            }
        }
        
        // End slide if maximum duration has passed
        if (SlideTimeRemaining <= 0)
        {
            EndSlide();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Dashing Implementation

bool URMCMovementComponent::PerformDash()
{
    if (!CanDash())
    {
        return false;
    }
    
    // Set dashing state
    bIsDashing = true;
    
    // Calculate dash direction
    FVector InputVector = ConsumeInputVector();
    if (InputVector.SizeSquared() > 0.1f)
    {
        // Dash in input direction
        DashDirection = InputVector.GetSafeNormal();
    }
    else
    {
        // Dash forward if no input
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (Character)
        {
            DashDirection = Character->GetActorForwardVector();
        }
        else
        {
            DashDirection = Velocity.GetSafeNormal();
            if (DashDirection.SizeSquared() < 0.1f)
            {
                DashDirection = FVector(1, 0, 0);
            }
        }
    }
    
    // Set custom movement mode
    SetMovementMode(MOVE_Custom, CMOVE_Dashing);
    
    // Apply initial dash velocity
    float DashSpeed = DashDistance / DashDuration;
    Velocity = DashDirection * DashSpeed;
    
    // Start dash timeout
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_DashTimeout,
        [this]() {
            bIsDashing = false;
            
            // Apply speed boost after dash
            if (IsMovingOnGround())
            {
                Velocity += DashDirection * DashGroundSpeedBoost;
            }
            else
            {
                Velocity += DashDirection * DashAirSpeedBoost;
            }
            
            // Return to appropriate movement mode
            if (IsMovingOnGround())
            {
                SetMovementMode(MOVE_Walking);
            }
            else
            {
                SetMovementMode(MOVE_Falling);
            }
            
            // Broadcast events
            OnDashEnd.Broadcast();
            OnDashEnd_BP();
        },
        DashDuration,
        false
    );
    
    // Set cooldown
    DashCooldownRemaining = DashCooldown;
    
    // Add momentum
    AddMomentum(20.0f);
    
    // Broadcast events
    OnDashBegin.Broadcast(DashDirection);
    OnDashBegin_BP(DashDirection); // Using DashDir parameter name in BP event
    
    return true;
}

bool URMCMovementComponent::CanDash() const
{
    // Check cooldown
    if (DashCooldownRemaining > 0)
    {
        return false;
    }
    
    // Check if already dashing
    if (bIsDashing)
    {
        return false;
    }
    
    // Check if we have enough momentum
    if (!HasMinimumMomentumForAction(MaxMomentum * 0.3f))
    {
        return false;
    }
    
    return true;
}

float URMCMovementComponent::GetDashCooldownPercent() const
{
    if (DashCooldown <= 0)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(DashCooldownRemaining / DashCooldown, 0.0f, 1.0f);
}

void URMCMovementComponent::ApplyDashForces(float DeltaTime)
{
    // During dash, maintain constant velocity in dash direction
    float DashSpeed = DashDistance / DashDuration;
    Velocity = DashDirection * DashSpeed;
}

void URMCMovementComponent::UpdateDashCooldown(float DeltaTime)
{
    if (DashCooldownRemaining > 0)
    {
        DashCooldownRemaining -= DeltaTime;
        if (DashCooldownRemaining < 0)
        {
            DashCooldownRemaining = 0;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// Double Jump Implementation

bool URMCMovementComponent::PerformDoubleJump()
{
    if (!CanDoubleJump())
    {
        return false;
    }
    
    // Set double jump state
    bHasDoubleJumped = true;
    
    // Apply double jump velocity
    Velocity.Z = DoubleJumpZVelocity;
    
    // Add momentum
    AddMomentum(10.0f);
    
    // Broadcast event
    OnDoubleJump_BP();
    
    return true;
}

bool URMCMovementComponent::CanDoubleJump() const
{
    // Must be falling (in air)
    if (!IsFalling())
    {
        return false;
    }
    
    // Must not have already used double jump
    if (bHasDoubleJumped)
    {
        return false;
    }
    
    // Check if we have enough momentum
    if (!HasMinimumMomentumForAction(MaxMomentum * 0.2f))
    {
        return false;
    }
    
    return true;
}

void URMCMovementComponent::ResetJumpState()
{
    // Reset double jump state when landing
    if (IsMovingOnGround())
    {
        bHasDoubleJumped = false;
    }
}

//////////////////////////////////////////////////////////////////////////
// Momentum System Implementation

void URMCMovementComponent::UpdateMomentum(float DeltaTime)
{
    float PreviousMomentum = CurrentMomentum;
    
    // Build momentum when moving at high speeds
    if (Velocity.SizeSquared() > FMath::Square(MaxWalkSpeed * 1.2f))
    {
        AddMomentum(MomentumBuildRate * DeltaTime);
    }
    // Decay momentum when moving slowly or not moving
    else if (Velocity.SizeSquared() < FMath::Square(MaxWalkSpeed * 0.5f))
    {
        ReduceMomentum(MomentumDecayRate * DeltaTime);
    }
    
    // Broadcast momentum changed event if it changed significantly
    if (FMath::Abs(PreviousMomentum - CurrentMomentum) > 0.1f)
    {
        OnMomentumChanged.Broadcast(CurrentMomentum);
    }
}

float URMCMovementComponent::GetMomentumPercentage() const
{
    return FMath::Clamp(CurrentMomentum / MaxMomentum, 0.0f, 1.0f);
}

//////////////////////////////////////////////////////////////////////////
// Debug Helper Functions

void URMCMovementComponent::DebugWallRunning(bool bEnableLogging, bool bDrawDebugLines, float LineDuration)
{
    if (bEnableLogging)
    {
        LogWallRunningState();
    }
    
    if (bDrawDebugLines)
    {
        DrawWallRunDebugHelpers(LineDuration);
    }
}

FString URMCMovementComponent::GetMovementStateDebugString() const
{
    FString StateString = TEXT("Movement State: ");
    
    if (bIsWallRunning)
    {
        StateString += TEXT("Wall Running");
    }
    else if (bIsSliding)
    {
        StateString += TEXT("Sliding");
    }
    else if (bIsDashing)
    {
        StateString += TEXT("Dashing");
    }
    else if (IsFalling())
    {
        StateString += TEXT("Falling");
    }
    else if (IsMovingOnGround())
    {
        StateString += TEXT("Walking");
    }
    else
    {
        StateString += TEXT("Unknown");
    }
    
    // Add speed information
    float CurrentSpeed = Velocity.Size();
    float SpeedPercent = (GlobalSpeedCap > 0.0f) ? (CurrentSpeed / GlobalSpeedCap) * 100.0f : 0.0f;
    
    StateString += FString::Printf(TEXT("\nSpeed: %.1f (%.1f%% of cap)"), 
        CurrentSpeed, SpeedPercent);
    
    return StateString;
}

void URMCMovementComponent::LogWallRunningState() const
{
    if (!bIsWallRunning)
    {
        UE_LOG(LogTemp, Display, TEXT("Not currently wall running"));
        
        // Check if wall running is possible
        FVector WallNormal;
        bool bFoundWall = FindWallRunSurface(WallNormal);
        
        UE_LOG(LogTemp, Display, TEXT("Wall found: %s"), bFoundWall ? TEXT("Yes") : TEXT("No"));
        if (bFoundWall)
        {
            UE_LOG(LogTemp, Display, TEXT("Wall Normal: X=%f, Y=%f, Z=%f"), 
                WallNormal.X, WallNormal.Y, WallNormal.Z);
        }
        
        UE_LOG(LogTemp, Display, TEXT("Current Velocity: %f"), Velocity.Size());
        UE_LOG(LogTemp, Display, TEXT("Current Momentum: %f / %f"), CurrentMomentum, MaxMomentum);
        UE_LOG(LogTemp, Display, TEXT("Is Moving On Ground: %s"), IsMovingOnGround() ? TEXT("Yes") : TEXT("No"));
        
        return;
    }
    
    // Log wall running state
    UE_LOG(LogTemp, Display, TEXT("=== Wall Running Debug Info ==="));
    UE_LOG(LogTemp, Display, TEXT("Wall Normal: X=%f, Y=%f, Z=%f"), 
        CurrentWallNormal.X, CurrentWallNormal.Y, CurrentWallNormal.Z);
    
    FVector WallRunDir = GetWallRunDirection();
    UE_LOG(LogTemp, Display, TEXT("Wall Run Direction: X=%f, Y=%f, Z=%f"), 
        WallRunDir.X, WallRunDir.Y, WallRunDir.Z);
    
    UE_LOG(LogTemp, Display, TEXT("Current Velocity: X=%f, Y=%f, Z=%f (Magnitude: %f)"), 
        Velocity.X, Velocity.Y, Velocity.Z, Velocity.Size());
    
    UE_LOG(LogTemp, Display, TEXT("Wall Run Speed: %f"), WallRunSpeed);
    UE_LOG(LogTemp, Display, TEXT("Wall Run Time Remaining: %f / %f"), WallRunTimeRemaining, MaxWallRunTime);
    UE_LOG(LogTemp, Display, TEXT("Wall Run Control Multiplier: %f"), WallRunControlMultiplier);
    UE_LOG(LogTemp, Display, TEXT("Wall Attraction Force: %f"), WallAttractionForce);
    
    // Get character input - can't use ConsumeInputVector in const method
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (Character)
    {
        // Just log that we can't get input in a const method
        UE_LOG(LogTemp, Display, TEXT("Input Vector: Cannot access in const method"));
    }
    
    UE_LOG(LogTemp, Display, TEXT("=============================="));
}

void URMCMovementComponent::DrawWallRunDebugHelpers(float LineDuration) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }
    
    FVector CharacterLocation = Character->GetActorLocation();
    
    // Draw current velocity
    DrawDebugLine(
        World,
        CharacterLocation,
        CharacterLocation + Velocity * 0.1f, // Scale for visibility
        FColor::Green,
        false,
        LineDuration,
        0,
        3.0f
    );
    
    // If wall running, draw wall normal and run direction
    if (bIsWallRunning)
    {
        // Draw wall normal
        DrawDebugLine(
            World,
            CharacterLocation,
            CharacterLocation + CurrentWallNormal * 100.0f,
            FColor::Red,
            false,
            LineDuration,
            0,
            3.0f
        );
        
        // Draw wall run direction
        FVector WallRunDir = GetWallRunDirection();
        DrawDebugLine(
            World,
            CharacterLocation,
            CharacterLocation + WallRunDir * 100.0f,
            FColor::Blue,
            false,
            LineDuration,
            0,
            3.0f
        );
        
        // Draw text for wall run speed
        DrawDebugString(
            World,
            CharacterLocation + FVector(0, 0, 100),
            FString::Printf(TEXT("Wall Run Speed: %.1f"), Velocity.Size()),
            nullptr,
            FColor::White,
            LineDuration
        );
    }
    else
    {
        // Check if wall running is possible
        FVector WallNormal;
        bool bFoundWall = FindWallRunSurface(WallNormal);
        
        if (bFoundWall)
        {
            // Draw potential wall normal
            DrawDebugLine(
                World,
                CharacterLocation,
                CharacterLocation + WallNormal * 100.0f,
                FColor::Yellow,
                false,
                LineDuration,
                0,
                3.0f
            );
            
            // Draw potential wall run direction
            FVector PotentialWallRunDir = FVector::CrossProduct(WallNormal, FVector(0, 0, 1)).GetSafeNormal();
            DrawDebugLine(
                World,
                CharacterLocation,
                CharacterLocation + PotentialWallRunDir * 100.0f,
                FColor::Cyan,
                false,
                LineDuration,
                0,
                3.0f
            );
        }
    }
    
    // Draw movement state text
    DrawDebugString(
        World,
        CharacterLocation + FVector(0, 0, 120),
        GetMovementStateDebugString(),
        nullptr,
        FColor::White,
        LineDuration
    );
}

FVector URMCMovementComponent::GetWallRunDirection() const
{
    if (!bIsWallRunning || CurrentWallNormal.IsZero())
    {
        return FVector::ZeroVector;
    }
    
    FVector WallRunDirection = FVector::CrossProduct(CurrentWallNormal, FVector(0, 0, 1)).GetSafeNormal();
    
    // Make sure we're running in the correct direction along the wall
    if (FVector::DotProduct(WallRunDirection, Velocity) < 0)
    {
        WallRunDirection = -WallRunDirection;
    }
    
    return WallRunDirection;
}

bool URMCMovementComponent::IsWallRunningPossible() const
{
    // Check all conditions for wall running
    
    // Can't wall run if on ground
    if (IsMovingOnGround())
    {
        return false;
    }
    
    // Can't wall run if not moving fast enough
    if (Velocity.SizeSquared() < 10000.0f) // 100 units/sec
    {
        return false;
    }
    
    // Check if we have enough momentum
    if (!HasMinimumMomentumForAction(MaxMomentum * 0.2f))
    {
        return false;
    }
    
    // Check if there's a valid wall to run on
    FVector WallNormal;
    return FindWallRunSurface(WallNormal);
}

void URMCMovementComponent::ForceWallRunSpeed(float SpeedMultiplier)
{
    if (!bIsWallRunning)
    {
        return;
    }
    
    FVector WallRunDir = GetWallRunDirection();
    Velocity = WallRunDir * WallRunSpeed * SpeedMultiplier;
    
    // Log the forced speed
    UE_LOG(LogTemp, Display, TEXT("Forced wall run speed to %f"), Velocity.Size());
}

void URMCMovementComponent::SetSpeedCapSettings(float NewSpeedCap, float NewDamping, bool bApplyToZ)
{
    GlobalSpeedCap = FMath::Max(0.0f, NewSpeedCap);
    SpeedCapDamping = FMath::Clamp(NewDamping, 0.0f, 1.0f);
    bApplySpeedCapToZVelocity = bApplyToZ;
    
    UE_LOG(LogTemp, Display, TEXT("Speed Cap Settings Updated: Cap=%.1f, Damping=%.2f, ApplyToZ=%s"), 
        GlobalSpeedCap, SpeedCapDamping, bApplySpeedCapToZVelocity ? TEXT("True") : TEXT("False"));
}

//////////////////////////////////////////////////////////////////////////
// IRMCMomentumBased Interface Implementation

float URMCMovementComponent::GetCurrentMomentum_Implementation() const
{
    return CurrentMomentum;
}

void URMCMovementComponent::AddMomentum_Implementation(float Amount)
{
    CurrentMomentum = FMath::Clamp(CurrentMomentum + Amount, 0.0f, MaxMomentum);
    OnMomentumChanged.Broadcast(CurrentMomentum);
}

void URMCMovementComponent::ReduceMomentum_Implementation(float Amount)
{
    CurrentMomentum = FMath::Clamp(CurrentMomentum - Amount, 0.0f, MaxMomentum);
    OnMomentumChanged.Broadcast(CurrentMomentum);
}

bool URMCMovementComponent::HasMinimumMomentumForAction_Implementation(float RequiredMomentum) const
{
    return CurrentMomentum >= RequiredMomentum;
}

float URMCMovementComponent::GetMomentumPercent_Implementation() const
{
    return GetMomentumPercentage();
}