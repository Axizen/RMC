// Fill out your copyright notice in the Description page of Project Settings.

#include "HeavyRangedWeapon.h"
#include "RangedWeaponProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Rift/RiftComponent.h"
#include "../MomentumComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UHeavyRangedWeapon::UHeavyRangedWeapon()
{
    // Set default values for R&C-style weapon
    RaritaniumPoints = 0;
    ElementalType = EElementalType::None;
    ElementalEffectChance = 0.25f;
    ElementalDamageMultiplier = 1.5f;
    
    // Area effect defaults
    bHasAreaEffect = false;
    AreaEffectRadius = 300.0f;
    AreaDamageMultiplier = 0.5f;
    
    // Alternative fire mode
    AltFireMode = EAltFireMode::Charge;
    AltFireCooldown = 3.0f;
    RemainingAltFireCooldown = 0.0f;
    
    // Ammo regeneration
    bCanRegenerateAmmo = true;
    AmmoRegenerationRate = 1.0f;
    AmmoRegenerationDelay = 2.0f;
    
    // Set base weapon properties
    BaseDamage = 30.0f;
    FireRate = 0.5f;
    MaxAmmo = 20;
    CurrentAmmo = MaxAmmo;
    ReloadTime = 2.0f;
    Range = 1500.0f;
    
    // Style system integration
    StylePointsPerHit = 15.0f;
    
    // Rift system integration
    MomentumGainPerHit = 8.0f;
    bCanFireDuringRift = false;
    
    // Set current state
    CurrentState = EWeaponState::Idle;
    
    // Enable tick for ammo regeneration and cooldowns
    PrimaryComponentTick.bCanEverTick = true;
    
    // Initialize evolution data
    EvolutionData.EvolvedName = FText::FromString("Evolved Heavy Weapon");
    EvolutionData.EvolvedDescription = FText::FromString("An evolved version with enhanced capabilities");
    EvolutionData.RequiredLevel = 5;
    EvolutionData.bHasEvolved = false;
    EvolutionData.DamageMultiplier = 1.5f;
    EvolutionData.RangeMultiplier = 1.2f;
    EvolutionData.FireRateMultiplier = 1.0f;
    EvolutionData.AmmoCapacityMultiplier = 1.5f;
}

void UHeavyRangedWeapon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Handle ammo regeneration
    if (bCanRegenerateAmmo && CurrentAmmo < MaxAmmo && CurrentState != EWeaponState::Reloading)
    {
        static float TimeSinceLastShot = 0.0f;
        
        if (CurrentState == EWeaponState::Idle)
        {
            TimeSinceLastShot += DeltaTime;
            
            if (TimeSinceLastShot >= AmmoRegenerationDelay)
            {
                // Regenerate ammo based on rate and delta time
                float AmmoToAdd = AmmoRegenerationRate * DeltaTime;
                float FractionalAmmo = FMath::Frac(AmmoToAdd);
                int32 WholeAmmo = FMath::FloorToInt(AmmoToAdd);
                
                // Add whole ammo immediately
                CurrentAmmo = FMath::Min(CurrentAmmo + WholeAmmo, MaxAmmo);
                
                // Handle fractional ammo with a random chance
                if (FMath::FRand() < FractionalAmmo)
                {
                    CurrentAmmo = FMath::Min(CurrentAmmo + 1, MaxAmmo);
                }
            }
        }
        else
        {
            // Reset timer when firing
            TimeSinceLastShot = 0.0f;
        }
    }
    
    // Handle alt fire cooldown
    if (RemainingAltFireCooldown > 0.0f)
    {
        RemainingAltFireCooldown = FMath::Max(RemainingAltFireCooldown - DeltaTime, 0.0f);
    }
}

bool UHeavyRangedWeapon::PurchaseUpgrade(int32 UpgradeIndex)
{
    // Check if upgrade index is valid
    if (UpgradeIndex < 0 || UpgradeIndex >= UpgradeGrid.Num())
    {
        return false;
    }
    
    // Get the upgrade
    FWeaponUpgrade& Upgrade = UpgradeGrid[UpgradeIndex];
    
    // Check if already purchased
    if (Upgrade.bPurchased)
    {
        return false;
    }
    
    // Check if we have enough Raritanium points
    if (RaritaniumPoints < Upgrade.Cost)
    {
        return false;
    }
    
    // Check if all prerequisites are met
    for (int32 PrereqIndex : Upgrade.Prerequisites)
    {
        if (PrereqIndex >= 0 && PrereqIndex < UpgradeGrid.Num() && !UpgradeGrid[PrereqIndex].bPurchased)
        {
            return false;
        }
    }
    
    // Purchase the upgrade
    Upgrade.bPurchased = true;
    RaritaniumPoints -= Upgrade.Cost;
    
    // Apply upgrade effects
    ApplyUpgradeEffects(Upgrade);
    
    return true;
}

void UHeavyRangedWeapon::EvolveWeapon()
{
    // Check if we can evolve
    if (EvolutionData.bHasEvolved || WeaponLevel < EvolutionData.RequiredLevel)
    {
        return;
    }
    
    // Evolve the weapon
    EvolutionData.bHasEvolved = true;
    
    // Apply evolution stat boosts
    BaseDamage *= EvolutionData.DamageMultiplier;
    Range *= EvolutionData.RangeMultiplier;
    FireRate /= EvolutionData.FireRateMultiplier; // Lower is faster
    MaxAmmo = FMath::RoundToInt(MaxAmmo * EvolutionData.AmmoCapacityMultiplier);
    CurrentAmmo = MaxAmmo; // Refill ammo on evolution
    
    // Update projectile class if evolved version is specified
    if (EvolutionData.EvolvedProjectileClass)
    {
        ProjectileClass = EvolutionData.EvolvedProjectileClass;
    }
    
    // Update weapon name and description
    WeaponName = EvolutionData.EvolvedName;
    WeaponDescription = EvolutionData.EvolvedDescription;
}

void UHeavyRangedWeapon::ApplyElementalEffect(AActor* Target)
{
    // Only apply if we have an elemental type
    if (ElementalType == EElementalType::None || !Target)
    {
        return;
    }
    
    // Apply different effects based on elemental type
    switch (ElementalType)
    {
        case EElementalType::Fire:
            // Apply fire damage over time
            // This would typically be implemented with a GameplayEffect or custom damage over time component
            UGameplayStatics::ApplyDamage(
                Target,
                BaseDamage * 0.2f * ElementalDamageMultiplier,
                GetOwner()->GetInstigatorController(),
                GetOwner(),
                UDamageType::StaticClass()
            );
            break;
            
        case EElementalType::Ice:
            // Slow the target
            // This would typically be implemented by applying a movement speed debuff
            if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
            {
                if (TargetCharacter->GetCharacterMovement())
                {
                    TargetCharacter->GetCharacterMovement()->MaxWalkSpeed *= 0.5f;
                    
                    // Set a timer to restore normal speed
                    FTimerHandle TimerHandle;
                    FTimerDelegate TimerDelegate;
                    TimerDelegate.BindLambda([TargetCharacter]()
                    {
                        if (TargetCharacter && TargetCharacter->GetCharacterMovement())
                        {
                            TargetCharacter->GetCharacterMovement()->MaxWalkSpeed *= 2.0f; // Restore normal speed
                        }
                    });
                    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 3.0f, false);
                }
            }
            break;
            
        case EElementalType::Electric:
            {
                // Chain damage to nearby enemies
                TArray<AActor*> NearbyActors;
                UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
            
            for (AActor* NearbyActor : NearbyActors)
            {
                if (NearbyActor != Target && NearbyActor != GetOwner())
                {
                    float Distance = FVector::Distance(Target->GetActorLocation(), NearbyActor->GetActorLocation());
                    if (Distance <= 300.0f) // Chain lightning range
                    {
                        UGameplayStatics::ApplyDamage(
                            NearbyActor,
                            BaseDamage * 0.3f * ElementalDamageMultiplier,
                            GetOwner()->GetInstigatorController(),
                            GetOwner(),
                            UDamageType::StaticClass()
                        );
                    }
                }
            }
            break;
            }
            
        case EElementalType::Poison:
            // Apply poison damage over time
            // This would typically be implemented with a GameplayEffect or custom damage over time component
            for (int i = 0; i < 3; i++) // Apply damage 3 times
            {
                FTimerHandle TimerHandle;
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindLambda([this, Target]()
                {
                    if (IsValid(Target))
                    {
                        UGameplayStatics::ApplyDamage(
                            Target,
                            BaseDamage * 0.15f * ElementalDamageMultiplier,
                            GetOwner()->GetInstigatorController(),
                            GetOwner(),
                            UDamageType::StaticClass()
                        );
                    }
                });
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.0f * (i + 1), false);
            }
            break;
            
        default:
            break;
    }
}

void UHeavyRangedWeapon::StrafeFire(FVector Direction)
{
    // This implements strafing while firing, a common R&C mechanic
    
    // Get owner character
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter || CurrentState != EWeaponState::Idle)
    {
        return;
    }
    
    // Store original fire rate
    float OriginalFireRate = FireRate;
    
    // Increase fire rate during strafe
    FireRate *= 0.7f; // 30% faster firing while strafing
    
    // Fire weapon
    FireWeapon();
    
    // Restore original fire rate
    FireRate = OriginalFireRate;
}

void UHeavyRangedWeapon::CreateAreaEffect(FVector Location, float Radius)
{
    // Only process if area effects are enabled
    if (!bHasAreaEffect)
    {
        return;
    }
    
    // Apply radial damage
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(GetOwner());
    
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        BaseDamage * AreaDamageMultiplier,
        Location,
        Radius > 0.0f ? Radius : AreaEffectRadius,
        UDamageType::StaticClass(),
        IgnoredActors,
        GetOwner(),
        GetOwner()->GetInstigatorController(),
        true // Apply full damage at center, fall off towards edges
    );
    
    // Spawn visual effect
    if (MuzzleEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            MuzzleEffect,
            Location,
            FRotator::ZeroRotator,
            FVector(Radius / 100.0f) // Scale effect based on radius
        );
    }
    
    // Play sound effect
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, Location);
    }
}

bool UHeavyRangedWeapon::FireWeapon()
{
    // Use base implementation with R&C-specific additions
    bool bSuccess = Super::FireWeapon();
    
    if (bSuccess)
    {
        // Apply area effect if enabled
        if (bHasAreaEffect)
        {
            // Get owner character for spawn location
            ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
            if (OwnerCharacter)
            {
                // Get camera view point for projectile direction
                FVector CameraLocation;
                FRotator CameraRotation;
                if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
                {
                    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
                }
                else
                {
                    CameraLocation = OwnerCharacter->GetActorLocation();
                    CameraRotation = OwnerCharacter->GetActorRotation();
                }
                
                // Calculate impact location (simplified - would normally use line trace)
                FVector ImpactLocation = CameraLocation + (CameraRotation.Vector() * Range);
                
                // Create area effect at impact location
                CreateAreaEffect(ImpactLocation, AreaEffectRadius);
            }
        }
    }
    
    return bSuccess;
}

bool UHeavyRangedWeapon::AltFireWeapon()
{
    // Check if we're on cooldown
    if (RemainingAltFireCooldown > 0.0f)
    {
        return false;
    }
    
    // Check if we can fire
    if (CurrentState != EWeaponState::Idle || CurrentAmmo <= 0)
    {
        return false;
    }
    
    // Handle different alt fire modes
    switch (AltFireMode)
    {
        case EAltFireMode::Charge:
            // Use base charge implementation
            return Super::AltFireWeapon();
            
        case EAltFireMode::Burst:
            // Fire multiple shots in quick succession
            for (int i = 0; i < 3; i++)
            {
                FTimerHandle TimerHandle;
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindLambda([this]()
                {
                    if (CurrentAmmo > 0)
                    {
                        Super::FireWeapon();
                    }
                });
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f * i, false);
            }
            break;
            
        case EAltFireMode::Spread:
            // Fire multiple projectiles in a spread pattern
            {
                // Get owner character for spawn location
                ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
                if (!OwnerCharacter)
                {
                    return false;
                }
                
                // Get camera view point for projectile direction
                FVector CameraLocation;
                FRotator CameraRotation;
                if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
                {
                    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
                }
                else
                {
                    CameraLocation = OwnerCharacter->GetActorLocation();
                    CameraRotation = OwnerCharacter->GetActorRotation();
                }
                
                // Calculate muzzle location
                FVector MuzzleLocation = CameraLocation + CameraRotation.Vector() * 100.0f;
                
                // Fire 5 projectiles in a spread pattern
                for (int i = -2; i <= 2; i++)
                {
                    // Calculate spread rotation
                    FRotator SpreadRotation = CameraRotation + FRotator(0, i * 10.0f, 0);
                    
                    // Spawn projectile with spread
                    SpawnProjectile(MuzzleLocation, SpreadRotation);
                }
                
                // Consume ammo (5 shots worth)
                ConsumeAmmo(5);
                
                // Play effects
                PlayWeaponEffects(MuzzleEffect, FireSound);
            }
            break;
            
        case EAltFireMode::Elemental:
            // Temporarily change elemental type for next shot
            {
                // Store original elemental type
                EElementalType OriginalType = ElementalType;
                
                // Cycle to next elemental type
                switch (ElementalType)
                {
                    case EElementalType::None:
                        ElementalType = EElementalType::Fire;
                        break;
                    case EElementalType::Fire:
                        ElementalType = EElementalType::Ice;
                        break;
                    case EElementalType::Ice:
                        ElementalType = EElementalType::Electric;
                        break;
                    case EElementalType::Electric:
                        ElementalType = EElementalType::Poison;
                        break;
                    case EElementalType::Poison:
                        ElementalType = EElementalType::Fire;
                        break;
                }
                
                // Fire with new elemental type
                Super::FireWeapon();
                
                // Restore original elemental type after a delay
                FTimerHandle TimerHandle;
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindLambda([this, OriginalType]()
                {
                    ElementalType = OriginalType;
                });
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 5.0f, false);
            }
            break;
            
        default:
            return false;
    }
    
    // Start cooldown
    RemainingAltFireCooldown = AltFireCooldown;
    
    // Set timer to complete cooldown
    GetWorld()->GetTimerManager().SetTimer(
        AltFireCooldownTimerHandle,
        this,
        &UHeavyRangedWeapon::CompleteAltFireCooldown,
        AltFireCooldown,
        false
    );
    
    return true;
}

void UHeavyRangedWeapon::LevelUpWeapon()
{
    // Call base implementation
    Super::LevelUpWeapon();
    
    // Check if we should evolve
    if (WeaponLevel == EvolutionData.RequiredLevel && !EvolutionData.bHasEvolved)
    {
        EvolveWeapon();
    }
}

void UHeavyRangedWeapon::StartReload()
{
    // R&C weapons often have instant reloads or no reloads
    // This implementation provides a faster reload than the base class
    
    // Check if we can reload
    if (CurrentState != EWeaponState::Idle || CurrentAmmo == MaxAmmo)
    {
        return;
    }
    
    // Set state to reloading
    CurrentState = EWeaponState::Reloading;
    
    // Play reload sound
    if (ReloadSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), ReloadSound);
    }
    
    // Set timer for reload completion (faster than base class)
    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &URangedWeaponBase::FinishReload,
        ReloadTime * 0.7f, // 30% faster reload
        false
    );
}

void UHeavyRangedWeapon::RegenerateAmmo()
{
    // This is called by a timer to regenerate ammo over time
    if (CurrentAmmo < MaxAmmo && CurrentState != EWeaponState::Reloading)
    {
        CurrentAmmo = FMath::Min(CurrentAmmo + 1, MaxAmmo);
        
        // Set timer for next regeneration
        if (CurrentAmmo < MaxAmmo)
        {
            GetWorld()->GetTimerManager().SetTimer(
                AmmoRegenerationTimerHandle,
                this,
                &UHeavyRangedWeapon::RegenerateAmmo,
                1.0f / AmmoRegenerationRate,
                false
            );
        }
    }
}

void UHeavyRangedWeapon::CompleteAltFireCooldown()
{
    // Reset cooldown
    RemainingAltFireCooldown = 0.0f;
}

void UHeavyRangedWeapon::ApplyUpgradeEffects(const FWeaponUpgrade& Upgrade)
{
    // Apply stat modifiers
    if (Upgrade.DamageModifier != 0.0f)
    {
        BaseDamage *= (1.0f + Upgrade.DamageModifier);
    }
    
    if (Upgrade.RangeModifier != 0.0f)
    {
        Range *= (1.0f + Upgrade.RangeModifier);
    }
    
    if (Upgrade.FireRateModifier != 0.0f)
    {
        // Lower fire rate value means faster firing
        FireRate /= (1.0f + Upgrade.FireRateModifier);
    }
    
    if (Upgrade.AmmoCapacityModifier != 0.0f)
    {
        int32 OldMaxAmmo = MaxAmmo;
        MaxAmmo = FMath::RoundToInt(MaxAmmo * (1.0f + Upgrade.AmmoCapacityModifier));
        
        // Increase current ammo proportionally
        CurrentAmmo = FMath::RoundToInt(CurrentAmmo * (float)MaxAmmo / OldMaxAmmo);
    }
    
    if (Upgrade.ReloadTimeModifier != 0.0f)
    {
        // Lower reload time value means faster reloading
        ReloadTime /= (1.0f + Upgrade.ReloadTimeModifier);
    }
    
    // Apply special effect unlocks
    if (Upgrade.bUnlocksElementalEffect)
    {
        ElementalType = Upgrade.ElementalType;
    }
    
    if (Upgrade.bUnlocksProjectileTransform)
    {
        // This would typically modify the projectile class or add a transform modifier
        // For now, just enable area effect if it's an explosive transform
        if (Upgrade.ProjectileTransform == EProjectileTransform::Explosive)
        {
            bHasAreaEffect = true;
        }
    }
}

int32 UHeavyRangedWeapon::GetUpgradeLevel(EUpgradeType UpgradeType) const
{
    // Count how many upgrades of this type have been purchased
    int32 Level = 0;
    
    for (const FWeaponUpgrade& Upgrade : UpgradeGrid)
    {
        if (Upgrade.bPurchased)
        {
            switch (UpgradeType)
            {
                case EUpgradeType::Damage:
                    if (Upgrade.DamageModifier > 0.0f)
                    {
                        Level++;
                    }
                    break;
                    
                case EUpgradeType::Range:
                    if (Upgrade.RangeModifier > 0.0f)
                    {
                        Level++;
                    }
                    break;
                    
                case EUpgradeType::FireRate:
                    if (Upgrade.FireRateModifier > 0.0f)
                    {
                        Level++;
                    }
                    break;
                    
                case EUpgradeType::AmmoCapacity:
                    if (Upgrade.AmmoCapacityModifier > 0.0f)
                    {
                        Level++;
                    }
                    break;
                    
                case EUpgradeType::ReloadTime:
                    if (Upgrade.ReloadTimeModifier > 0.0f)
                    {
                        Level++;
                    }
                    break;
                    
                case EUpgradeType::Elemental:
                    if (Upgrade.bUnlocksElementalEffect)
                    {
                        Level++;
                    }
                    break;
                    
                case EUpgradeType::Projectile:
                    if (Upgrade.bUnlocksProjectileTransform)
                    {
                        Level++;
                    }
                    break;
            }
        }
    }
    
    return Level;
}