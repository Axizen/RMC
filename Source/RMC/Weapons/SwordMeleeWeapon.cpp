// Fill out your copyright notice in the Description page of Project Settings.

#include "SwordMeleeWeapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "StyleComponent.h"
#include "../MomentumComponent.h"

USwordMeleeWeapon::USwordMeleeWeapon()
{
    // Set default values for sword properties
    WeaponType = EMeleeWeaponType::Sword;
    BaseDamage = 25.0f;
    AttackSpeed = 1.2f;
    WeaponReach = 200.0f;
    
    // DMC-specific properties
    bHasExceedSystem = true;
    MaxExceedLevel = 3;
    CurrentExceedLevel = 0;
    ExceedDamageMultiplier = 1.3f;
    ExceedDuration = 10.0f;
    PerfectExceedWindow = 0.1f;
    
    // Aerial combat properties
    bHasAerialRave = true;
    AerialDamageMultiplier = 1.2f;
    AerialStyleMultiplier = 1.5f;
    
    // Guard properties
    bCanGuard = true;
    bCanParry = true;
    ParryWindow = 0.2f;
    ParryDamageMultiplier = 2.0f;
    GuardDamageReduction = 0.7f;
    
    // State tracking
    bIsExceedActive = false;
    LastExceedAttemptTime = 0.0f;
    bInParryWindow = false;
}

// Called when the game starts
void USwordMeleeWeapon::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize weapon name and description
    if (WeaponName.IsEmpty())
    {
        WeaponName = FText::FromString("Red Queen");
    }
    
    if (WeaponDescription.IsEmpty())
    {
        WeaponDescription = FText::FromString("A customized sword with a motorcycle-like gear shift, allowing for Exceed attacks that deal extra damage.");
    }
}

// Called every frame
void USwordMeleeWeapon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Additional sword-specific tick logic can go here
}

// Attempt to use the Exceed system
bool USwordMeleeWeapon::AttemptExceed()
{
    if (!bHasExceedSystem || bIsExceedActive || CurrentState != EMeleeWeaponState::Idle)
    {
        return false;
    }
    
    // Record the attempt time for perfect timing checks
    LastExceedAttemptTime = GetWorld()->GetTimeSeconds();
    
    // Play exceed effect
    PlayExceedEffect();
    
    // Activate exceed with one level
    ActivateExceed(1);
    
    return true;
}

// Attempt a perfect MAX-Act (DMC4 Red Queen mechanic)
bool USwordMeleeWeapon::AttemptMaxAct()
{
    if (!bHasExceedSystem || CurrentState != EMeleeWeaponState::Attacking)
    {
        return false;
    }
    
    // Check if within the perfect timing window
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceAttack = CurrentTime - LastAttackInputTime;
    
    if (CheckPerfectTiming(TimeSinceAttack, 0.3f, PerfectExceedWindow))
    {
        // MAX-Act successful - activate max exceed
        PlayMaxActEffect();
        ActivateExceed(MaxExceedLevel);
        
        // Broadcast MAX-Act event
        OnMaxActPerformed.Broadcast();
        
        // Add style points for perfect timing
        if (StyleComponent)
        {
            StyleComponent->AddStylePoints(300.0f, FName("MaxAct"));
        }
        
        return true;
    }
    
    return false;
}

// Activate the Exceed system
void USwordMeleeWeapon::ActivateExceed(int32 Levels)
{
    if (!bHasExceedSystem)
    {
        return;
    }
    
    // Set exceed level (clamped to max)
    CurrentExceedLevel = FMath::Clamp(CurrentExceedLevel + Levels, 0, MaxExceedLevel);
    
    // Set exceed active state
    bIsExceedActive = (CurrentExceedLevel > 0);
    
    // Start exceed timer
    if (bIsExceedActive)
    {
        GetWorld()->GetTimerManager().SetTimer(ExceedTimerHandle, this, &USwordMeleeWeapon::OnExceedEnd, ExceedDuration, false);
        
        // Broadcast exceed activated event
        OnExceedActivated.Broadcast(CurrentExceedLevel);
    }
}

// Consume one level of Exceed for an attack
void USwordMeleeWeapon::ConsumeExceed()
{
    if (!bIsExceedActive || CurrentExceedLevel <= 0)
    {
        return;
    }
    
    // Reduce exceed level
    CurrentExceedLevel--;
    
    // Check if exceed is depleted
    if (CurrentExceedLevel <= 0)
    {
        bIsExceedActive = false;
        GetWorld()->GetTimerManager().ClearTimer(ExceedTimerHandle);
        
        // Broadcast exceed depleted event
        OnExceedDepleted.Broadcast();
    }
}

// Attempt a parry (perfect guard)
bool USwordMeleeWeapon::AttemptParry()
{
    if (!bCanParry || CurrentState != EMeleeWeaponState::Idle)
    {
        return false;
    }
    
    // Enter parry window
    bInParryWindow = true;
    
    // Start parry window timer
    GetWorld()->GetTimerManager().SetTimer(ParryWindowTimerHandle, this, &USwordMeleeWeapon::OnParryWindowEnd, ParryWindow, false);
    
    // Also start guarding
    AttemptGuard();
    
    return true;
}

// Perform High Time (launcher move)
void USwordMeleeWeapon::PerformHighTime()
{
    // This is a special move that launches enemies upward
    PerformAttack(EAttackInput::Up);
}

// Perform Split Second (forward dash attack)
void USwordMeleeWeapon::PerformSplitSecond()
{
    // This is a special move that performs a forward dash attack
    PerformAttack(EAttackInput::Forward);
}

// Perform Round Trip (throwing sword move)
void USwordMeleeWeapon::PerformRoundTrip()
{
    // This is a special move that throws the sword and returns it
    PerformAttack(EAttackInput::Special);
}

// Override base attack function with sword-specific behavior
bool USwordMeleeWeapon::PerformAttack(EAttackInput AttackType)
{
    // Check for Exceed input
    if (AttackType == EAttackInput::Exceed)
    {
        return AttemptExceed();
    }
    
    // Perform the base attack
    bool bAttackStarted = Super::PerformAttack(AttackType);
    
    if (bAttackStarted && bIsExceedActive)
    {
        // If exceed is active, consume one level for this attack
        ConsumeExceed();
    }
    
    return bAttackStarted;
}

// Override combo continuation with sword-specific behavior
bool USwordMeleeWeapon::ContinueCombo(EAttackInput AttackType)
{
    // Check for MAX-Act timing if in a combo
    if (AttackType == EAttackInput::Exceed)
    {
        return AttemptMaxAct();
    }
    
    // Continue with base combo behavior
    return Super::ContinueCombo(AttackType);
}

// Override charge attack with sword-specific behavior
void USwordMeleeWeapon::ChargeAttack(float ChargeAmount)
{
    // Sword can use Exceed during charge for enhanced charge attacks
    Super::ChargeAttack(ChargeAmount);
}

// Override release charge attack with sword-specific behavior
void USwordMeleeWeapon::ReleaseChargeAttack()
{
    // If Exceed is active, the charged attack will be enhanced
    Super::ReleaseChargeAttack();
}

// Override guard with sword-specific behavior
bool USwordMeleeWeapon::AttemptGuard()
{
    // Sword can parry during guard start
    bool bGuardStarted = Super::AttemptGuard();
    
    if (bGuardStarted && bCanParry)
    {
        // Start parry window
        bInParryWindow = true;
        GetWorld()->GetTimerManager().SetTimer(ParryWindowTimerHandle, this, &USwordMeleeWeapon::OnParryWindowEnd, ParryWindow, false);
    }
    
    return bGuardStarted;
}

// Override damage application with sword-specific behavior
FMeleeHitData USwordMeleeWeapon::ApplyDamage(AActor* Target, const FHitResult& Hit)
{
    // Calculate base damage
    float FinalDamage = BaseDamage;
    
    // Apply combo node specific damage if in a combo
    if (ComboNodes.IsValidIndex(CurrentComboIndex))
    {
        FinalDamage = ComboNodes[CurrentComboIndex].Damage;
    }
    
    // Apply Exceed damage multiplier if active
    if (bIsExceedActive)
    {
        FinalDamage *= CalculateExceedDamageMultiplier();
    }
    
    // Apply aerial damage multiplier if in air
    if (IsInAir())
    {
        FinalDamage *= AerialDamageMultiplier;
    }
    
    // Create the hit data with the modified damage
    FMeleeHitData HitData = Super::ApplyDamage(Target, Hit);
    
    // Apply additional style points for aerial attacks
    if (IsInAir() && StyleComponent)
    {
        StyleComponent->AddStylePoints(100.0f * AerialStyleMultiplier, FName("AerialBonus"));
    }
    
    return HitData;
}

// Called when exceed timer ends
void USwordMeleeWeapon::OnExceedEnd()
{
    // Reset exceed state
    bIsExceedActive = false;
    CurrentExceedLevel = 0;
    
    // Broadcast exceed depleted event
    OnExceedDepleted.Broadcast();
}

// Called when parry window timer ends
void USwordMeleeWeapon::OnParryWindowEnd()
{
    bInParryWindow = false;
}

// Play the exceed effect
void USwordMeleeWeapon::PlayExceedEffect()
{
    if (!ExceedEffect)
    {
        return;
    }
    
    // Get the socket location for the effect
    if (OwnerMesh && !AttachSocketName.IsNone())
    {
        FVector SocketLocation = OwnerMesh->GetSocketLocation(AttachSocketName);
        FRotator SocketRotation = OwnerMesh->GetSocketRotation(AttachSocketName);
        
        // Spawn the effect
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ExceedEffect,
            SocketLocation,
            SocketRotation
        );
        
        // Play the sound if available
        if (ExceedSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExceedSound, SocketLocation);
        }
    }
}

// Play the MAX-Act effect
void USwordMeleeWeapon::PlayMaxActEffect()
{
    if (!MaxActEffect)
    {
        return;
    }
    
    // Get the socket location for the effect
    if (OwnerMesh && !AttachSocketName.IsNone())
    {
        FVector SocketLocation = OwnerMesh->GetSocketLocation(AttachSocketName);
        FRotator SocketRotation = OwnerMesh->GetSocketRotation(AttachSocketName);
        
        // Spawn the effect
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            MaxActEffect,
            SocketLocation,
            SocketRotation
        );
        
        // Play the sound if available
        if (MaxActSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), MaxActSound, SocketLocation);
        }
        
        // Add a camera shake for feedback
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            PC->ClientStartCameraShake(TSubclassOf<UCameraShakeBase>(UCameraShakeBase::StaticClass()), 1.5f);
        }
    }
}

// Calculate the current Exceed damage multiplier based on level
float USwordMeleeWeapon::CalculateExceedDamageMultiplier() const
{
    // Base multiplier plus additional per level
    return 1.0f + ((ExceedDamageMultiplier - 1.0f) * (float)CurrentExceedLevel / (float)MaxExceedLevel);
}

// Check if the character is in the air
bool USwordMeleeWeapon::IsInAir() const
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
        if (MovementComp)
        {
            return MovementComp->IsFalling();
        }
    }
    
    return false;
}