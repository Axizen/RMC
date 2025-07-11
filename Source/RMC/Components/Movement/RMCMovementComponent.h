// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Interfaces/RMCMomentumBased.h"
#include "RMCMovementComponent.generated.h"

// Forward declarations
class ACharacter;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallRunBegin, const FVector&, WallNormal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallRunEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideBegin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDashBegin, const FVector&, DashDirection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDashEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMomentumChanged, float, NewMomentum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhysicsProfileChanged, FName, ProfileName);

/**
 * Physics profile for movement component
 */
USTRUCT(BlueprintType)
struct FMovementPhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FName ProfileName = NAME_None;

    // Wall Running
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float WallRunSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float WallRunGravityScale = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float WallRunJumpOffForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float MinWallRunHeight = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float MaxWallRunTime = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float WallRunControlMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float WallAttractionForce = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
    float MaxWallRunSurfaceAngle = 11.0f;

    // Sliding
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding")
    float SlideSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding")
    float SlideFriction = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding")
    float SlideMinDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding")
    float SlideMaxDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding")
    float SlideMinSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding")
    float SlideDownhillAccelerationMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sliding")
    float SlideCapsuleHeightScale = 0.5f;

    // Dashing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dashing")
    float DashDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dashing")
    float DashDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dashing")
    float DashCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dashing")
    float DashGroundSpeedBoost = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dashing")
    float DashAirSpeedBoost = 300.0f;

    // Double Jump
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Double Jump")
    float DoubleJumpZVelocity = 600.0f;

    // Momentum
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
    float MomentumRetentionRate = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
    float MaxMomentum = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
    float MomentumDecayRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
    float MomentumBuildRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
    float MomentumSpeedMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum")
    float MomentumAccelerationMultiplier = 0.3f;
    
    // Speed Cap
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Cap")
    float GlobalSpeedCap = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Cap")
    float SpeedCapDamping = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Cap")
    bool bApplySpeedCapToZVelocity = false;
};

/**
 * Custom movement component for Titanfall-style momentum-based movement
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RMC_API URMCMovementComponent : public UCharacterMovementComponent, public IRMCMomentumBased
{
    GENERATED_BODY()

public:
    // Constructor
    URMCMovementComponent(const FObjectInitializer& ObjectInitializer);

    // Wall Running Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "200.0", UIMax = "1500.0", 
        ToolTip = "Maximum speed while wall running in units per second"))
    float WallRunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0", 
        ToolTip = "Gravity scale while wall running (0 = no gravity, 1 = normal gravity)"))
    float WallRunGravityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "100.0", UIMax = "1000.0", 
        ToolTip = "Force applied when jumping off a wall"))
    float WallRunJumpOffForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "200.0", 
        ToolTip = "Minimum height above ground required to wall run"))
    float MinWallRunHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "0.5", UIMax = "10.0", 
        ToolTip = "Maximum time in seconds a wall run can last"))
    float MaxWallRunTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0", 
        ToolTip = "How much player input affects wall run direction (0 = none, 1 = full control)"))
    float WallRunControlMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "500.0", 
        ToolTip = "Force pulling the character toward the wall"))
    float WallAttractionForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Wall Running", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "45.0", 
        ToolTip = "Maximum angle (in degrees) a surface can have from vertical to be considered a wall"))
    float MaxWallRunSurfaceAngle;

    // Sliding Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Sliding", 
        meta = (ClampMin = "0.0", UIMin = "200.0", UIMax = "2000.0", 
        ToolTip = "Initial speed when starting a slide"))
    float SlideSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Sliding", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0", 
        ToolTip = "Friction applied during sliding (higher = more slowdown)"))
    float SlideFriction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Sliding", 
        meta = (ClampMin = "0.0", UIMin = "0.1", UIMax = "1.0", 
        ToolTip = "Minimum duration in seconds a slide must last"))
    float SlideMinDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Sliding", 
        meta = (ClampMin = "0.0", UIMin = "0.5", UIMax = "5.0", 
        ToolTip = "Maximum duration in seconds a slide can last"))
    float SlideMaxDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Sliding", 
        meta = (ClampMin = "0.0", UIMin = "50.0", UIMax = "500.0", 
        ToolTip = "Minimum speed required to maintain a slide"))
    float SlideMinSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Sliding", 
        meta = (ClampMin = "0.0", UIMin = "1.0", UIMax = "5.0", 
        ToolTip = "Multiplier for downhill acceleration while sliding"))
    float SlideDownhillAccelerationMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Sliding", 
        meta = (ClampMin = "0.1", UIMin = "0.3", UIMax = "0.7", 
        ToolTip = "Scale factor for capsule height while sliding (0.5 = half height)"))
    float SlideCapsuleHeightScale;

    // Dashing Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Dashing", 
        meta = (ClampMin = "0.0", UIMin = "100.0", UIMax = "1000.0", 
        ToolTip = "Distance covered during a dash"))
    float DashDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Dashing", 
        meta = (ClampMin = "0.01", UIMin = "0.05", UIMax = "0.5", 
        ToolTip = "Duration of the dash in seconds"))
    float DashDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Dashing", 
        meta = (ClampMin = "0.0", UIMin = "0.1", UIMax = "5.0", 
        ToolTip = "Cooldown time between dashes in seconds"))
    float DashCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Dashing", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0", 
        ToolTip = "Speed boost applied after dashing on ground"))
    float DashGroundSpeedBoost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Dashing", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1000.0", 
        ToolTip = "Speed boost applied after dashing in air"))
    float DashAirSpeedBoost;

    // Double Jump Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Double Jump", 
        meta = (ClampMin = "0.0", UIMin = "300.0", UIMax = "1000.0", 
        ToolTip = "Upward velocity applied during double jump"))
    float DoubleJumpZVelocity;

    // Momentum Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Momentum", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0", 
        ToolTip = "How much momentum is retained when not actively building it"))
    float MomentumRetentionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Momentum", 
        meta = (ClampMin = "0.0", UIMin = "50.0", UIMax = "200.0", 
        ToolTip = "Maximum momentum value that can be accumulated"))
    float MaxMomentum;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Momentum", 
        meta = (ClampMin = "0.0", UIMin = "1.0", UIMax = "20.0", 
        ToolTip = "Rate at which momentum decays when moving slowly"))
    float MomentumDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Momentum", 
        meta = (ClampMin = "0.0", UIMin = "1.0", UIMax = "50.0", 
        ToolTip = "Rate at which momentum builds when moving quickly"))
    float MomentumBuildRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Momentum", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0", 
        ToolTip = "How much momentum affects maximum speed (higher = more effect)"))
    float MomentumSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Momentum", 
        meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "2.0", 
        ToolTip = "How much momentum affects acceleration (higher = more effect)"))
    float MomentumAccelerationMultiplier;

    // Physics Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics Profiles",
        meta = (ToolTip = "Collection of physics profiles that can be applied to the movement component"))
    TArray<FMovementPhysicsProfile> PhysicsProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics Profiles",
        meta = (ToolTip = "Currently active physics profile name"))
    FName CurrentProfileName;

    // Movement states
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    bool bIsWallRunning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    bool bIsSliding;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    bool bIsDashing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    bool bHasDoubleJumped;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    float CurrentMomentum;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    float DashCooldownRemaining;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    float WallRunTimeRemaining;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    float SlideTimeRemaining;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    FVector CurrentWallNormal;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|States")
    FVector DashDirection;

    // Blueprint events
    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnWallRunBegin OnWallRunBegin;

    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnWallRunEnd OnWallRunEnd;

    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnSlideBegin OnSlideBegin;

    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnSlideEnd OnSlideEnd;

    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnDashBegin OnDashBegin;

    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnDashEnd OnDashEnd;

    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnMomentumChanged OnMomentumChanged;

    UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
    FOnPhysicsProfileChanged OnPhysicsProfileChanged;

    // Blueprint callable functions for movement
    UFUNCTION(BlueprintCallable, Category = "Movement|Wall Running")
    void StartWallRun();

    UFUNCTION(BlueprintCallable, Category = "Movement|Wall Running")
    void EndWallRun();

    UFUNCTION(BlueprintCallable, Category = "Movement|Wall Running")
    bool CanWallRun() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Wall Running")
    void WallRunJump();

    UFUNCTION(BlueprintCallable, Category = "Movement|Sliding")
    void StartSlide();

    UFUNCTION(BlueprintCallable, Category = "Movement|Sliding")
    void EndSlide();

    UFUNCTION(BlueprintCallable, Category = "Movement|Sliding")
    bool CanSlide() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Dashing")
    bool PerformDash();

    UFUNCTION(BlueprintCallable, Category = "Movement|Dashing")
    bool CanDash() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Dashing")
    float GetDashCooldownPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Double Jump")
    bool PerformDoubleJump();

    UFUNCTION(BlueprintCallable, Category = "Movement|Double Jump")
    bool CanDoubleJump() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Momentum")
    void UpdateMomentum(float DeltaTime);

    // Non-interface version of GetMomentumPercent
    UFUNCTION(BlueprintCallable, Category = "Movement|Momentum")
    float GetMomentumPercentage() const;
    
    // Debug helper functions
    UFUNCTION(BlueprintCallable, Category = "Movement|Debug")
    void DebugWallRunning(bool bEnableLogging = true, bool bDrawDebugLines = true, float LineDuration = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Movement|Debug")
    FString GetMovementStateDebugString() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Debug")
    void LogWallRunningState() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Debug")
    void DrawWallRunDebugHelpers(float LineDuration = 0.1f) const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Debug")
    FVector GetWallRunDirection() const;

    UFUNCTION(BlueprintPure, Category = "Movement|Debug")
    bool IsWallRunningPossible() const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Debug")
    void ForceWallRunSpeed(float SpeedMultiplier = 1.2f);
    
    UFUNCTION(BlueprintCallable, Category = "Movement|Debug")
    void SetSpeedCapSettings(float NewSpeedCap, float NewDamping, bool bApplyToZ);

    // Blueprint callable functions for physics profiles
    UFUNCTION(BlueprintCallable, Category = "Movement|Physics Profiles",
        meta = (ToolTip = "Applies a named physics profile to the movement component"))
    bool SetMovementPhysicsProfile(FName ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Movement|Physics Profiles",
        meta = (ToolTip = "Resets all physics values to their defaults"))
    void ResetMovementPhysicsToDefaults();

    UFUNCTION(BlueprintCallable, Category = "Movement|Physics Profiles",
        meta = (ToolTip = "Creates a new physics profile with the current settings"))
    void SaveCurrentPhysicsAsProfile(FName ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Movement|Physics Profiles",
        meta = (ToolTip = "Returns all available physics profile names"))
    TArray<FName> GetAvailablePhysicsProfileNames() const;

    // Speed cap settings
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Speed Cap", 
    meta = (ClampMin = "0.0", UIMin = "1000.0", UIMax = "5000.0", 
    ToolTip = "Maximum speed cap for all movement modes"))
float GlobalSpeedCap;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Speed Cap", 
    meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0", 
    ToolTip = "How quickly to slow down when exceeding speed cap (0 = instant, 1 = very gradual)"))
float SpeedCapDamping;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics|Speed Cap", 
    meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "1.0", 
    ToolTip = "Whether to apply speed cap to Z velocity"))
bool bApplySpeedCapToZVelocity;

// Blueprint callable functions for specific physics settings
    UFUNCTION(BlueprintCallable, Category = "Movement|Physics|Wall Running",
        meta = (ToolTip = "Set wall running physics parameters"))
    void SetWallRunningPhysics(float Speed, float WallRunGravity, float JumpForce, float ControlMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Movement|Physics|Sliding",
        meta = (ToolTip = "Set sliding physics parameters"))
    void SetSlidingPhysics(float Speed, float Friction, float DownhillAcceleration, float CapsuleScale);

    UFUNCTION(BlueprintCallable, Category = "Movement|Physics|Dashing",
        meta = (ToolTip = "Set dashing physics parameters"))
    void SetDashingPhysics(float Distance, float Duration, float Cooldown, float GroundBoost, float AirBoost);

    UFUNCTION(BlueprintCallable, Category = "Movement|Physics|Momentum",
        meta = (ToolTip = "Set momentum physics parameters"))
    void SetMomentumPhysics(float MaxValue, float BuildRate, float DecayRate, float SpeedMultiplier, float AccelMultiplier);

    // Interface implementations
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
    float GetCurrentMomentum() const;
    virtual float GetCurrentMomentum_Implementation() const override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
    void AddMomentum(float Amount);
    virtual void AddMomentum_Implementation(float Amount) override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
    void ReduceMomentum(float Amount);
    virtual void ReduceMomentum_Implementation(float Amount) override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
    bool HasMinimumMomentumForAction(float RequiredMomentum) const;
    virtual bool HasMinimumMomentumForAction_Implementation(float RequiredMomentum) const override;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Movement|Momentum")
    float GetMomentumPercent() const;
    virtual float GetMomentumPercent_Implementation() const override;

    // Blueprint implementable events
    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Wall Running")
    void OnWallRunBegin_BP(const FVector& WallNormal);

    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Wall Running")
    void OnWallRunEnd_BP();

    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Sliding")
    void OnSlideBegin_BP();

    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Sliding")
    void OnSlideEnd_BP();

    // Fixed parameter name to avoid shadowing
    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Dashing")
    void OnDashBegin_BP(const FVector& DashDir);

    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Dashing")
    void OnDashEnd_BP();

    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Double Jump")
    void OnDoubleJump_BP();

    UFUNCTION(BlueprintImplementableEvent, Category = "Movement|Physics Profiles")
    void OnPhysicsProfileChanged_BP(FName ProfileName);

    // Override movement functions
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
    virtual float GetMaxSpeed() const override;
    virtual float GetMaxAcceleration() const override;

protected:
    virtual void PhysWalking(float deltaTime, int32 Iterations) override;
    virtual void PhysCustom(float deltaTime, int32 Iterations) override;
    virtual bool DoJump(bool bReplayingMoves) override;

    // Custom movement modes
    enum ECustomMovementMode
    {
        CMOVE_WallRunning = 0,
        CMOVE_Sliding = 1,
        CMOVE_Dashing = 2
    };

    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    bool FindWallRunSurface(FVector& OutWallNormal) const;

    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    void ApplyWallRunForces(float DeltaTime, const FVector& WallNormal);

    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    void ApplySlideForces(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    void ApplyDashForces(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    void UpdateDashCooldown(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    void UpdateWallRunTime(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    void UpdateSlideTime(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement|Utility")
    void ResetJumpState();

    // Default physics profile
    void InitializeDefaultPhysicsProfile();
    FMovementPhysicsProfile DefaultPhysicsProfile;

    // Timer handles
    FTimerHandle TimerHandle_WallRunTimeout;
    FTimerHandle TimerHandle_SlideTimeout;
    FTimerHandle TimerHandle_DashTimeout;
};