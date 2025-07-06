// Fill out your copyright notice in the Description page of Project Settings.

#include "PrecisionRangedWeapon.h"
#include "RangedWeaponProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Rift/RiftComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UPrecisionRangedWeapon::UPrecisionRangedWeapon()
{
    // Set default values for DMC-style weapon
    bAutoTargeting = true;
    StyleMultiplier = 1.2f;
    bCanJuggleEnemies = true;
    JuggleForce = 500.0f;
    
    // Rapid fire defaults
    bCanRapidFire = true;
    RapidFireRate = 0.1f;
    RapidFireBurstCount = 3;
    
    // Charge shot defaults
    bHasChargeShot = true;
    MaxChargeTime = 2.0f;
    ChargeMultiplier = 3.0f;
    CurrentChargeTime = 0.0f;
    bIsCharging = false;
    bIsEffectPlaying = false;
    
    // Set socket name
    SocketName = FName("MuzzleSocket");
    
    // Set base weapon properties
    BaseDamage = 15.0f;
    FireRate = 0.25f;
    MaxAmmo = 30;
    CurrentAmmo = MaxAmmo;
    ReloadTime = 1.2f;
    Range = 2000.0f;
    
    // Style system integration
    StylePointsPerHit = 50.0f;
    
    // Rift system integration
    MomentumGainPerHit = 5.0f;
    bCanFireDuringRift = true;
    
    // Set current state
    CurrentState = EWeaponState::Idle;
    
    // Enable tick for charge functionality
    PrimaryComponentTick.bCanEverTick = true;
}

void UPrecisionRangedWeapon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update charge time if charging
    if (bIsCharging)
    {
        CurrentChargeTime = FMath::Min(CurrentChargeTime + DeltaTime, MaxChargeTime);
    }
}

void UPrecisionRangedWeapon::PlayEffect(UNiagaraSystem* Effect, FVector Location, FRotator Rotation)
{
    if (Effect && GetWorld())
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Effect,
            Location,
            Rotation
        );
    }
}

void UPrecisionRangedWeapon::OnCooldownComplete()
{
    CurrentState = EWeaponState::Idle;
}

bool UPrecisionRangedWeapon::FireWeapon()
{
    // Check if we can fire
    if (CurrentState != EWeaponState::Idle || CurrentAmmo <= 0)
    {
        return false;
    }
    
    // Get owner character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return false;
    }
    
    // Get muzzle transform
    FVector MuzzleLocation;
    FRotator MuzzleRotation;
    if (OwnerCharacter->GetMesh() && !SocketName.IsNone())
    {
        MuzzleLocation = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
        MuzzleRotation = OwnerCharacter->GetMesh()->GetSocketRotation(SocketName);
    }
    else
    {
        // Fallback to character's location and rotation
        MuzzleLocation = OwnerCharacter->GetActorLocation() + 
                         (OwnerCharacter->GetActorForwardVector() * 50.0f) + 
                         (OwnerCharacter->GetActorUpVector() * 50.0f);
        MuzzleRotation = OwnerCharacter->GetActorRotation();
    }
    
    // Determine target and direction
    FVector TargetDirection;
    
    if (bAutoTargeting)
    {
        // Find best target for auto-targeting
        AActor* Target = FindBestTarget();
        
        if (Target)
        {
            // Calculate direction to target
            FVector TargetLocation = Target->GetActorLocation();
            TargetDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();
            MuzzleRotation = TargetDirection.Rotation();
        }
        else
        {
            // No target found, use forward vector
            TargetDirection = MuzzleRotation.Vector();
        }
    }
    else
    {
        // Use forward vector
        TargetDirection = MuzzleRotation.Vector();
    }
    
    // Spawn projectile
    if (ProjectileClass && GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerCharacter;
        SpawnParams.Instigator = OwnerCharacter;
        
        ARangedWeaponProjectile* Projectile = GetWorld()->SpawnActor<ARangedWeaponProjectile>(
            ProjectileClass, 
            MuzzleLocation, 
            MuzzleRotation, 
            SpawnParams);
        
        if (Projectile)
        {
            // Set projectile properties
            Projectile->Damage = BaseDamage;
            Projectile->StylePointsOnHit = StylePointsPerHit * StyleMultiplier;
            Projectile->MomentumGainOnHit = MomentumGainPerHit;
            
            // If the projectile can juggle enemies, set the flag
            if (bCanJuggleEnemies)
            {
                Projectile->bCanJuggleTarget = true;
                Projectile->JuggleForce = JuggleForce;
            }
        }
    }
    
    // Consume ammo
    CurrentAmmo--;
    
    // Play effects
    PlayWeaponEffects(MuzzleEffect, FireSound);
    
    // Set cooldown
    CurrentState = EWeaponState::Firing;
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UPrecisionRangedWeapon::OnCooldownComplete, FireRate, false);
    
    // Broadcast event
    OnWeaponFired.Broadcast(this);
    
    return true;
}

bool UPrecisionRangedWeapon::AltFireWeapon()
{
    // For DMC-style weapons, alt fire is typically a special move or charged shot
    if (bHasChargeShot && bIsCharging)
    {
        // Release charged shot
        ReleaseCharge();
        return true;
    }
    else if (bCanRapidFire)
    {
        // Start rapid fire
        StartRapidFire();
        return true;
    }
    
    return false;
}

void UPrecisionRangedWeapon::ChargeWeapon(float ChargeAmount)
{
    if (!bHasChargeShot || CurrentState != EWeaponState::Idle || CurrentAmmo <= 0)
    {
        return;
    }
    
    // Start charging
    bIsCharging = true;
    
    // Play charge effect if not already playing
    if (!bIsEffectPlaying && ChargeEffect)
    {
        ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
        if (OwnerCharacter && OwnerCharacter->GetMesh())
        {
            FVector EffectLocation = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
            PlayEffect(ChargeEffect, EffectLocation, OwnerCharacter->GetActorRotation());
            bIsEffectPlaying = true;
        }
    }
}

void UPrecisionRangedWeapon::ReleaseCharge()
{
    if (!bIsCharging || CurrentState != EWeaponState::Idle || CurrentAmmo <= 0)
    {
        return;
    }
    
    // Stop charging
    bIsCharging = false;
    
    // Get owner character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        CurrentChargeTime = 0.0f;
        return;
    }
    
    // Get muzzle transform
    FVector MuzzleLocation;
    FRotator MuzzleRotation;
    if (OwnerCharacter->GetMesh() && !SocketName.IsNone())
    {
        MuzzleLocation = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName);
        MuzzleRotation = OwnerCharacter->GetMesh()->GetSocketRotation(SocketName);
    }
    else
    {
        // Fallback to character's location and rotation
        MuzzleLocation = OwnerCharacter->GetActorLocation() + 
                         (OwnerCharacter->GetActorForwardVector() * 50.0f) + 
                         (OwnerCharacter->GetActorUpVector() * 50.0f);
        MuzzleRotation = OwnerCharacter->GetActorRotation();
    }
    
    // Calculate charge damage
    float ChargeDamage = CalculateChargeDamage();
    
    // Spawn charged projectile
    if (ProjectileClass && GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerCharacter;
        SpawnParams.Instigator = OwnerCharacter;
        
        ARangedWeaponProjectile* Projectile = GetWorld()->SpawnActor<ARangedWeaponProjectile>(
            ProjectileClass, 
            MuzzleLocation, 
            MuzzleRotation, 
            SpawnParams);
        
        if (Projectile)
        {
            // Set projectile properties with charge multiplier
            Projectile->Damage = ChargeDamage;
            Projectile->StylePointsOnHit = StylePointsPerHit * StyleMultiplier * (1.0f + CurrentChargeTime / MaxChargeTime);
            Projectile->MomentumGainOnHit = MomentumGainPerHit * (1.0f + CurrentChargeTime / MaxChargeTime);
            
            // Scale projectile size based on charge
            float ScaleFactor = 1.0f + (CurrentChargeTime / MaxChargeTime);
            Projectile->SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
            
            // If the projectile can juggle enemies, set the flag with increased force
            if (bCanJuggleEnemies)
            {
                Projectile->bCanJuggleTarget = true;
                Projectile->JuggleForce = JuggleForce * ScaleFactor;
            }
        }
    }
    
    // Consume ammo
    CurrentAmmo--;
    
    // Play effects
    PlayWeaponEffects(MuzzleEffect, FireSound);
    
    // Set cooldown
    CurrentState = EWeaponState::Firing;
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UPrecisionRangedWeapon::OnCooldownComplete, FireRate * 1.5f, false);
    
    // Reset charge time
    CurrentChargeTime = 0.0f;
    bIsEffectPlaying = false;
    
    // Broadcast event
    OnWeaponFired.Broadcast(this);
}

void UPrecisionRangedWeapon::OnRiftBegin()
{
    // DMC-style weapons can often be used during special moves
    // Enhance weapon properties during rift
    if (bCanFireDuringRift)
    {
        // Reduce cooldown during rift
        FireRate *= 0.7f;
        
        // Increase style points during rift
        StyleMultiplier *= 1.5f;
    }
}

void UPrecisionRangedWeapon::PerformGunslinger(EDirection InputDirection)
{
    // This would implement special moves based on directional input
    // Similar to Dante's Gunslinger style in DMC
    
    // Get owner character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter || CurrentState != EWeaponState::Idle)
    {
        return;
    }
    
    // Different special moves based on direction
    switch (InputDirection)
    {
        case EDirection::Forward:
            // Forward special - e.g., Honeycomb Fire (rapid forward shots)
            if (bCanRapidFire)
            {
                RapidFireBurstCount = 5;
                StartRapidFire();
            }
            break;
            
        case EDirection::Backward:
            // Backward special - e.g., Backslide (dodge while firing)
            if (OwnerCharacter)
            {
                // Move character backward while firing
                FVector BackwardDir = -OwnerCharacter->GetActorForwardVector();
                OwnerCharacter->LaunchCharacter(BackwardDir * 500.0f, true, true);
                
                // Fire weapon
                FireWeapon();
            }
            break;
            
        case EDirection::Left:
        case EDirection::Right:
            // Side special - e.g., Rain Storm (aerial 360 shots)
            if (OwnerCharacter && OwnerCharacter->GetCharacterMovement() && OwnerCharacter->GetCharacterMovement()->IsFalling())
            {
                // Spin character and fire multiple shots
                RapidFireBurstCount = 8;
                StartRapidFire();
            }
            break;
            
        case EDirection::Up:
            // Up special - e.g., Fireworks (upward spread shots)
            if (ProjectileClass && GetWorld())
            {
                // Spawn multiple projectiles in upward arc
                for (int32 i = 0; i < 5; i++)
                {
                    FVector MuzzleLocation = OwnerCharacter->GetActorLocation() + 
                                            (OwnerCharacter->GetActorForwardVector() * 50.0f) + 
                                            (OwnerCharacter->GetActorUpVector() * 50.0f);
                    
                    // Calculate spread rotation
                    float SpreadAngle = -30.0f + (i * 15.0f);
                    FRotator SpreadRotation = OwnerCharacter->GetActorRotation() + FRotator(SpreadAngle, 0.0f, 0.0f);
                    
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.Owner = OwnerCharacter;
                    SpawnParams.Instigator = OwnerCharacter;
                    
                    ARangedWeaponProjectile* Projectile = GetWorld()->SpawnActor<ARangedWeaponProjectile>(
                        ProjectileClass, 
                        MuzzleLocation, 
                        SpreadRotation, 
                        SpawnParams);
                    
                    if (Projectile)
                    {
                        Projectile->Damage = BaseDamage * 0.7f;
                        Projectile->StylePointsOnHit = StylePointsPerHit * StyleMultiplier * 1.2f;
                    }
                }
                
                // Consume ammo
                CurrentAmmo = FMath::Max(CurrentAmmo - 5, 0);
                
                // Play effects
                PlayWeaponEffects(MuzzleEffect, FireSound);
                
                // Set cooldown
                CurrentState = EWeaponState::Firing;
                GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UPrecisionRangedWeapon::OnCooldownComplete, FireRate * 2.0f, false);
            }
            break;
            
        case EDirection::Down:
            // Down special - e.g., Ground Trick (area effect)
            if (OwnerCharacter && !OwnerCharacter->GetCharacterMovement()->IsFalling())
            {
                // Create area effect around character
                FVector Location = OwnerCharacter->GetActorLocation();
                
                // Play special effect
                if (SpecialEffect && GetWorld())
                {
                    PlayEffect(SpecialEffect, Location, FRotator::ZeroRotator);
                }
                
                // Find enemies in radius
                TArray<AActor*> OverlappingActors;
                UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), OverlappingActors);
                
                for (AActor* Actor : OverlappingActors)
                {
                    float Distance = FVector::Distance(Location, Actor->GetActorLocation());
                    if (Distance <= 300.0f)
                    {
                        // Apply damage to nearby enemies
                        UGameplayStatics::ApplyDamage(Actor, BaseDamage * 1.5f, OwnerCharacter->GetController(), 
                                                    OwnerCharacter, UDamageType::StaticClass());
                        
                        // Apply knockback
                        if (Actor->GetRootComponent())
                        {
                            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                            if (PrimComp && PrimComp->IsSimulatingPhysics())
                            {
                                FVector KnockbackDir = (Actor->GetActorLocation() - Location).GetSafeNormal();
                                PrimComp->AddImpulse(KnockbackDir * 1000.0f, NAME_None, true);
                            }
                        }
                    }
                }
                
                // Consume ammo
                CurrentAmmo = FMath::Max(CurrentAmmo - 3, 0);
                
                // Set cooldown
                CurrentState = EWeaponState::Firing;
                GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UPrecisionRangedWeapon::OnCooldownComplete, FireRate * 2.5f, false);
            }
            break;
            
        default:
            // Default to normal fire
            FireWeapon();
            break;
    }
}

void UPrecisionRangedWeapon::CancelAttackWithGun()
{
    // This implements the DMC technique of canceling animations with gunfire
    // Typically used to reset attack animations or maintain air time
    
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return;
    }
    
    // Fire a single shot with reduced cooldown
    if (CurrentState == EWeaponState::Idle && CurrentAmmo > 0)
    {
        // Store original fire rate
        float OriginalFireRate = FireRate;
        
        // Reduce fire rate for cancel shot
        FireRate *= 0.5f;
        
        // Fire weapon
        FireWeapon();
        
        // Restore original fire rate
        FireRate = OriginalFireRate;
        
        // Add style points for technical execution
        UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientMessage(TEXT("Cancel Shot! +Style"));
    }
}

void UPrecisionRangedWeapon::JuggleTarget(AActor* Target)
{
    if (!Target || !bCanJuggleEnemies)
    {
        return;
    }
    
    // Apply upward force to keep target in air
    UPrimitiveComponent* TargetPrimitive = Cast<UPrimitiveComponent>(Target->GetRootComponent());
    if (TargetPrimitive && TargetPrimitive->IsSimulatingPhysics())
    {
        FVector UpwardForce = FVector(0, 0, JuggleForce);
        TargetPrimitive->AddImpulse(UpwardForce, NAME_None, true);
    }
    else
    {
        // For non-physics actors, try to use launch character
        ACharacter* TargetCharacter = Cast<ACharacter>(Target);
        if (TargetCharacter)
        {
            FVector UpwardForce = FVector(0, 0, JuggleForce);
            TargetCharacter->LaunchCharacter(UpwardForce, false, true);
        }
    }
}

void UPrecisionRangedWeapon::StartRapidFire()
{
    if (!bCanRapidFire || CurrentState != EWeaponState::Idle || CurrentAmmo <= 0)
    {
        return;
    }
    
    // Start rapid fire timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(RapidFireTimerHandle, this, &UPrecisionRangedWeapon::HandleRapidFire, RapidFireRate, true);
    }
}

void UPrecisionRangedWeapon::StopRapidFire()
{
    // Clear rapid fire timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(RapidFireTimerHandle);
    }
}

void UPrecisionRangedWeapon::HandleRapidFire()
{
    // Fire weapon
    if (CurrentAmmo > 0 && CurrentState == EWeaponState::Idle)
    {
        FireWeapon();
        
        // Decrement burst count
        RapidFireBurstCount--;
        
        // Stop if we've reached the burst limit or are out of ammo
        if (RapidFireBurstCount <= 0 || CurrentAmmo <= 0)
        {
            StopRapidFire();
        }
    }
    else
    {
        // Stop rapid fire if we can't fire
        StopRapidFire();
    }
}

AActor* UPrecisionRangedWeapon::FindBestTarget() const
{
    // Find the best target for auto-targeting
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter || !GetWorld())
    {
        return nullptr;
    }
    
    // Get owner's location and forward vector
    FVector OwnerLocation = OwnerCharacter->GetActorLocation();
    FVector OwnerForward = OwnerCharacter->GetActorForwardVector();
    
    // Find all potential targets
    TArray<AActor*> PotentialTargets;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), PotentialTargets);
    
    // If no targets, return nullptr
    if (PotentialTargets.Num() == 0)
    {
        return nullptr;
    }
    
    // Find best target based on distance and alignment with forward vector
    AActor* BestTarget = nullptr;
    float BestScore = -1.0f;
    
    for (AActor* Target : PotentialTargets)
    {
        // Skip invalid targets
        if (!Target || Target == OwnerCharacter)
        {
            continue;
        }
        
        // Calculate distance and direction to target
        FVector TargetLocation = Target->GetActorLocation();
        FVector ToTarget = TargetLocation - OwnerLocation;
        float Distance = ToTarget.Size();
        
        // Skip if too far
        if (Distance > Range)
        {
            continue;
        }
        
        // Normalize direction
        ToTarget.Normalize();
        
        // Calculate dot product to see how closely it aligns with our view
        float DotProduct = FVector::DotProduct(OwnerForward, ToTarget);
        
        // Skip if behind us or too far to the side
        if (DotProduct < 0.5f) // Roughly 60 degree cone
        {
            continue;
        }
        
        // Score based on alignment and distance (prefer closer targets)
        float Score = DotProduct * (1.0f - (Distance / Range));
        
        if (Score > BestScore)
        {
            BestScore = Score;
            BestTarget = Target;
        }
    }
    
    return BestTarget;
}

float UPrecisionRangedWeapon::CalculateChargeDamage() const
{
    // Calculate damage based on charge level
    float ChargeRatio = FMath::Clamp(CurrentChargeTime / MaxChargeTime, 0.0f, 1.0f);
    float DamageMultiplier = 1.0f + (ChargeRatio * (ChargeMultiplier - 1.0f));
    
    return BaseDamage * DamageMultiplier;
}