// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeWeaponBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "StyleComponent.h"
#include "../MomentumComponent.h"

// Sets default values for this component's properties
UMeleeWeaponBase::UMeleeWeaponBase()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize properties
    BaseDamage = 20.0f;
    AttackSpeed = 1.0f;
    WeaponReach = 150.0f;
    WeaponType = EMeleeWeaponType::None;
    CurrentState = EMeleeWeaponState::Idle;
    
    // Combo system
    CurrentComboIndex = -1;
    ComboCount = 0;
    ComboTimeWindow = 1.2f;
    
    // Style integration
    StylePointsPerHit = 100.0f;
    StyleMultiplier = 1.0f;
    
    // Momentum integration
    MomentumGainPerHit = 5.0f;
    MomentumDamageMultiplier = 0.1f;
    
    // Advanced mechanics
    bCanJumpCancel = true;
    bCanGuard = true;
    GuardDamageReduction = 0.5f;
    bHasAerialCombos = true;
    
    // State tracking
    bIsAttacking = false;
    bCanContinueCombo = false;
    bIsCharging = false;
    CurrentChargeAmount = 0.0f;
    bIsGuarding = false;
    bLastHitSuccessful = false;
    LastAttackInputTime = 0.0f;
}

// Called when the game starts
void UMeleeWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    // Get owner components
    AActor* Owner = GetOwner();
    if (Owner)
    {
        ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
        if (OwnerCharacter)
        {
            OwnerMesh = OwnerCharacter->GetMesh();
            if (OwnerMesh)
            {
                OwnerAnimInstance = OwnerMesh->GetAnimInstance();
            }
        }
        
        StyleComponent = GetOwnerStyleComponent();
        MomentumComponent = GetOwnerMomentumComponent();
    }
}

// Called every frame
void UMeleeWeaponBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Handle charging if active
    if (bIsCharging)
    {
        CurrentChargeAmount = FMath::Min(CurrentChargeAmount + DeltaTime, 1.0f);
    }
}

// Perform an attack with the specified input type
bool UMeleeWeaponBase::PerformAttack(EAttackInput AttackType)
{
    if (!CanAttack())
    {
        return false;
    }
    
    // Reset combo if not in a combo or can't continue
    if (CurrentComboIndex == -1 || !bCanContinueCombo)
    {
        ResetCombo();
        CurrentComboIndex = 0; // Start with first attack
    }
    else
    {
        // Continue combo with the specified attack type
        return ContinueCombo(AttackType);
    }
    
    // Play the animation for the current combo node
    if (OwnerAnimInstance && ComboNodes.IsValidIndex(CurrentComboIndex) && ComboNodes[CurrentComboIndex].ComboAnimation)
    {
        // Record the attack input time for perfect timing checks
        LastAttackInputTime = GetWorld()->GetTimeSeconds();
        
        // Start the animation montage
        float PlayRate = AttackSpeed;
        OwnerAnimInstance->Montage_Play(ComboNodes[CurrentComboIndex].ComboAnimation, PlayRate);
        
        // Set up the montage ended delegate
        if (!OwnerAnimInstance->OnMontageEnded.IsAlreadyBound(this, &UMeleeWeaponBase::OnMontageEnded))
        {
            OwnerAnimInstance->OnMontageEnded.AddDynamic(this, &UMeleeWeaponBase::OnMontageEnded);
        }
        
        // Update state
        bIsAttacking = true;
        CurrentState = EMeleeWeaponState::Attacking;
        
        // Start combo timer
        GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &UMeleeWeaponBase::OnComboTimeout, ComboTimeWindow, false);
        
        // Increment combo count
        ComboCount++;
        OnComboProgressed.Broadcast(ComboCount);
        
        return true;
    }
    
    return false;
}

// Continue a combo with the specified attack type
bool UMeleeWeaponBase::ContinueCombo(EAttackInput AttackType)
{
    if (!bCanContinueCombo || !ComboNodes.IsValidIndex(CurrentComboIndex))
    {
        return false;
    }
    
    // Check if this attack type has a valid next combo index
    if (!ComboNodes[CurrentComboIndex].NextComboIndices.Contains(AttackType))
    {
        return false;
    }
    
    // Get the next combo index
    int32 NextComboIndex = ComboNodes[CurrentComboIndex].NextComboIndices[AttackType];
    if (!ComboNodes.IsValidIndex(NextComboIndex))
    {
        return false;
    }
    
    // Update current combo index
    CurrentComboIndex = NextComboIndex;
    
    // Play the animation for the next combo node
    if (OwnerAnimInstance && ComboNodes[CurrentComboIndex].ComboAnimation)
    {
        // Record the attack input time for perfect timing checks
        LastAttackInputTime = GetWorld()->GetTimeSeconds();
        
        // Start the animation montage
        float PlayRate = AttackSpeed;
        OwnerAnimInstance->Montage_Play(ComboNodes[CurrentComboIndex].ComboAnimation, PlayRate);
        
        // Update state
        bIsAttacking = true;
        bCanContinueCombo = false;
        CurrentState = EMeleeWeaponState::Attacking;
        
        // Reset combo timer
        GetWorld()->GetTimerManager().ClearTimer(ComboTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &UMeleeWeaponBase::OnComboTimeout, ComboTimeWindow, false);
        
        // Increment combo count
        ComboCount++;
        OnComboProgressed.Broadcast(ComboCount);
        
        return true;
    }
    
    return false;
}

// Reset the combo state
void UMeleeWeaponBase::ResetCombo()
{
    CurrentComboIndex = -1;
    ComboCount = 0;
    bCanContinueCombo = false;
    bIsAttacking = false;
    GetWorld()->GetTimerManager().ClearTimer(ComboTimerHandle);
    CurrentState = EMeleeWeaponState::Idle;
    
    OnComboReset.Broadcast();
}

// Charge an attack
void UMeleeWeaponBase::ChargeAttack(float ChargeAmount)
{
    if (bIsAttacking || bIsGuarding)
    {
        return;
    }
    
    bIsCharging = true;
    CurrentChargeAmount = ChargeAmount;
    CurrentState = EMeleeWeaponState::Charging;
}

// Release a charged attack
void UMeleeWeaponBase::ReleaseChargeAttack()
{
    if (!bIsCharging)
    {
        return;
    }
    
    bIsCharging = false;
    
    // Perform an attack with charge modifier
    if (CurrentChargeAmount >= 0.5f) // Require at least 50% charge
    {
        // Use heavy attack for charged attacks
        PerformAttack(EAttackInput::Heavy);
    }
    else
    {
        // Not enough charge, perform normal attack
        PerformAttack(EAttackInput::Light);
    }
    
    CurrentChargeAmount = 0.0f;
}

// Perform a special attack with directional input
void UMeleeWeaponBase::PerformSpecialAttack(EAttackInput AttackType, EDirection Direction)
{
    // Default implementation just maps to standard attacks based on direction
    switch (Direction)
    {
        case EDirection::Forward:
            PerformAttack(EAttackInput::Forward);
            break;
        case EDirection::Backward:
            PerformAttack(EAttackInput::Back);
            break;
        case EDirection::Up:
            PerformAttack(EAttackInput::Up);
            break;
        case EDirection::Down:
            PerformAttack(EAttackInput::Down);
            break;
        default:
            PerformAttack(AttackType);
            break;
    }
}

// Attempt to guard/block
bool UMeleeWeaponBase::AttemptGuard()
{
    if (!bCanGuard || bIsAttacking || bIsCharging)
    {
        return false;
    }
    
    bIsGuarding = true;
    CurrentState = EMeleeWeaponState::Guarding;
    
    OnGuardBegin.Broadcast();
    
    return true;
}

// End guarding
void UMeleeWeaponBase::EndGuard()
{
    if (!bIsGuarding)
    {
        return;
    }
    
    bIsGuarding = false;
    CurrentState = EMeleeWeaponState::Idle;
    
    OnGuardEnd.Broadcast();
}

// Attempt a jump cancel
bool UMeleeWeaponBase::AttemptJumpCancel()
{
    if (!bCanJumpCancel || !bIsAttacking || !bLastHitSuccessful)
    {
        return false;
    }
    
    // Check if current combo node allows jump canceling
    if (ComboNodes.IsValidIndex(CurrentComboIndex) && !ComboNodes[CurrentComboIndex].bCanJumpCancel)
    {
        return false;
    }
    
    // Cancel current attack animation
    if (OwnerAnimInstance)
    {
        OwnerAnimInstance->Montage_Stop(0.1f);
    }
    
    // Reset attack state but maintain combo count
    bIsAttacking = false;
    bCanContinueCombo = true;
    CurrentState = EMeleeWeaponState::Idle;
    
    // Trigger jump on owner character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerCharacter->Jump();
        return true;
    }
    
    return false;
}

// Check if currently in a combo
bool UMeleeWeaponBase::IsInCombo() const
{
    return CurrentComboIndex != -1 && (bIsAttacking || bCanContinueCombo);
}

// Check if can attack
bool UMeleeWeaponBase::CanAttack() const
{
    // Can't attack if charging or guarding
    if (bIsCharging || bIsGuarding)
    {
        return false;
    }
    
    // Can attack if not attacking or if in a combo and can continue
    return !bIsAttacking || bCanContinueCombo;
}

// Check for hits
void UMeleeWeaponBase::CheckHit()
{
    if (!OwnerMesh)
    {
        return;
    }
    
    // Get the socket transform for hit detection
    FTransform SocketTransform = OwnerMesh->GetSocketTransform(AttachSocketName);
    FVector TraceStart = SocketTransform.GetLocation();
    
    // Get forward vector for trace direction
    FVector ForwardVector = SocketTransform.GetRotation().GetForwardVector();
    FVector TraceEnd = TraceStart + (ForwardVector * WeaponReach);
    
    // Setup trace parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    
    // Perform the trace
    TArray<FHitResult> HitResults;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(30.0f); // Adjust radius as needed
    
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        TraceStart,
        TraceEnd,
        FQuat::Identity,
        ECC_Pawn,
        SphereShape,
        QueryParams
    );
    
    bLastHitSuccessful = false;
    
    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (Hit.GetActor() && Hit.GetActor() != GetOwner())
            {
                // Apply damage to the hit actor
                FMeleeHitData HitData = ApplyDamage(Hit.GetActor(), Hit);
                
                // Broadcast hit event
                OnEnemyHit.Broadcast(HitData);
                
                // Play impact effects
                PlayImpactEffects(Hit);
                
                // Set last hit successful for jump canceling
                bLastHitSuccessful = true;
                
                // Check for perfect timing
                if (ComboNodes.IsValidIndex(CurrentComboIndex) && ComboNodes[CurrentComboIndex].bHasPerfectTimingWindow)
                {
                    float CurrentTime = GetWorld()->GetTimeSeconds();
                    float TimeSinceInput = CurrentTime - LastAttackInputTime;
                    
                    if (CheckPerfectTiming(TimeSinceInput, ComboNodes[CurrentComboIndex].PerfectTimingWindow, 0.1f))
                    {
                        OnPerfectTimingPerformed.Broadcast(CurrentComboIndex);
                    }
                }
            }
        }
    }
}

// Apply damage to a hit target
FMeleeHitData UMeleeWeaponBase::ApplyDamage(AActor* Target, const FHitResult& Hit)
{
    if (!Target)
    {
        return FMeleeHitData();
    }
    
    // Calculate base damage
    float FinalDamage = BaseDamage;
    
    // Apply combo node specific damage if in a combo
    if (ComboNodes.IsValidIndex(CurrentComboIndex))
    {
        FinalDamage = ComboNodes[CurrentComboIndex].Damage;
    }
    
    // Apply momentum damage multiplier if available
    if (MomentumComponent)
    {
        float MomentumMultiplier = 1.0f + (MomentumComponent->GetMomentumRatio() * MomentumDamageMultiplier);
        FinalDamage *= MomentumMultiplier;
        
        // Additional multiplier during overcharge
        if (MomentumComponent->bIsOvercharged)
        {
            FinalDamage *= MomentumComponent->OverchargeDamageMultiplier;
        }
    }
    
    // Check for critical hit (simple implementation - can be expanded)
    bool bCriticalHit = (FMath::RandRange(0.0f, 1.0f) < 0.1f); // 10% critical chance
    if (bCriticalHit)
    {
        FinalDamage *= 1.5f; // 50% bonus damage on critical
    }
    
    // Check for perfect timing bonus
    bool bPerfectTiming = false;
    if (ComboNodes.IsValidIndex(CurrentComboIndex) && ComboNodes[CurrentComboIndex].bHasPerfectTimingWindow)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float TimeSinceInput = CurrentTime - LastAttackInputTime;
        
        if (CheckPerfectTiming(TimeSinceInput, ComboNodes[CurrentComboIndex].PerfectTimingWindow, 0.1f))
        {
            FinalDamage *= ComboNodes[CurrentComboIndex].PerfectTimingDamageMultiplier;
            bPerfectTiming = true;
        }
    }
    
    // Apply the damage
    UGameplayStatics::ApplyPointDamage(
        Target,
        FinalDamage,
        Hit.TraceStart - Hit.TraceEnd,
        Hit,
        GetOwner()->GetInstigatorController(),
        GetOwner(),
        UDamageType::StaticClass()
    );
    
    // Add momentum if available
    if (MomentumComponent)
    {
        float MomentumGain = MomentumGainPerHit;
        
        // Apply combo node specific momentum gain if in a combo
        if (ComboNodes.IsValidIndex(CurrentComboIndex))
        {
            MomentumGain = ComboNodes[CurrentComboIndex].MomentumGain;
        }
        
        MomentumComponent->AddMomentum(MomentumGain, FName(TEXT("MeleeHit")));
    }
    
    // Add style points if available
    if (StyleComponent)
    {
        float StylePoints = StylePointsPerHit;
        
        // Apply combo node specific style points if in a combo
        if (ComboNodes.IsValidIndex(CurrentComboIndex))
        {
            StylePoints = ComboNodes[CurrentComboIndex].StylePoints;
        }
        
        // Bonus style points for combo count
        StylePoints *= (1.0f + (ComboCount * 0.1f)); // 10% bonus per combo hit
        
        // Additional bonus for perfect timing
        if (bPerfectTiming)
        {
            StylePoints *= 1.5f;
        }
        
        StyleComponent->AddStylePoints(StylePoints, FName(*FString::Printf(TEXT("MeleeHit_%d"), ComboCount)));
    }
    
    // Create and return hit data
    return FMeleeHitData(Target, Hit, FinalDamage, bCriticalHit, bPerfectTiming);
}

// Called when combo timer expires
void UMeleeWeaponBase::OnComboTimeout()
{
    ResetCombo();
}

// Called when animation montage ends
void UMeleeWeaponBase::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // If the combo was interrupted, reset it
    if (bInterrupted)
    {
        ResetCombo();
        return;
    }
    
    // If we're still in an attack state, allow combo continuation
    if (bIsAttacking)
    {
        bIsAttacking = false;
        bCanContinueCombo = true;
        CurrentState = EMeleeWeaponState::Idle;
    }
}

// Get the owner's style component
UStyleComponent* UMeleeWeaponBase::GetOwnerStyleComponent() const
{
    if (GetOwner())
    {
        return GetOwner()->FindComponentByClass<UStyleComponent>();
    }
    return nullptr;
}

// Get the owner's momentum component
UMomentumComponent* UMeleeWeaponBase::GetOwnerMomentumComponent() const
{
    if (GetOwner())
    {
        return GetOwner()->FindComponentByClass<UMomentumComponent>();
    }
    return nullptr;
}

// Play impact effects at hit location
void UMeleeWeaponBase::PlayImpactEffects(const FHitResult& Hit)
{
    // Get the impact effect from the current combo node if available
    UNiagaraSystem* ImpactEffect = nullptr;
    USoundBase* ImpactSound = nullptr;
    
    if (ComboNodes.IsValidIndex(CurrentComboIndex))
    {
        ImpactEffect = ComboNodes[CurrentComboIndex].ImpactEffect;
        ImpactSound = ComboNodes[CurrentComboIndex].ImpactSound;
    }
    
    // Play the impact effect if available
    if (ImpactEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ImpactEffect,
            Hit.Location,
            Hit.Normal.Rotation()
        );
    }
    
    // Play the impact sound if available
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            ImpactSound,
            Hit.Location
        );
    }
    
    // Add a small camera shake for feedback
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PC->ClientStartCameraShake(TSubclassOf<UCameraShakeBase>(UCameraShakeBase::StaticClass()), 1.0f);
    }
}

// Check if an input time falls within a perfect timing window
bool UMeleeWeaponBase::CheckPerfectTiming(float InputTime, float IdealTime, float Window) const
{
    return FMath::Abs(InputTime - IdealTime) <= Window;
}