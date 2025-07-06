// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeaponBase.h"
#include "WeaponEnums.h"
#include "WeaponStructs.h"
#include "HeavyRangedWeapon.generated.h"

/**
 * Heavy Ranged Weapon class inspired by Ratchet & Clank's weapon system.
 * Focuses on weapon evolution, upgrades, and elemental effects.
 */
UCLASS(Blueprintable)
class RMC_API UHeavyRangedWeapon : public URangedWeaponBase
{
    GENERATED_BODY()
    
public:
    UHeavyRangedWeapon();
    
    // R&C-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    TArray<FWeaponUpgrade> UpgradeGrid;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    int32 RaritaniumPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    EElementalType ElementalType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    float ElementalEffectChance;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    float ElementalDamageMultiplier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    FWeaponEvolution EvolutionData;
    
    // Area effect properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    bool bHasAreaEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    float AreaEffectRadius;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    float AreaDamageMultiplier;
    
    // Alternative fire mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    EAltFireMode AltFireMode;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    float AltFireCooldown;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|R&C")
    float RemainingAltFireCooldown;
    
    // Ammo regeneration (common in R&C games)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    bool bCanRegenerateAmmo;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    float AmmoRegenerationRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    float AmmoRegenerationDelay;
    
    // R&C-style functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|R&C")
    virtual bool PurchaseUpgrade(int32 UpgradeIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|R&C")
    virtual void EvolveWeapon();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|R&C")
    virtual void ApplyElementalEffect(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|R&C")
    virtual void StrafeFire(FVector Direction);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|R&C")
    virtual void CreateAreaEffect(FVector Location, float Radius);
    
    // Override base functions with R&C-specific behavior
    virtual bool FireWeapon() override;
    virtual bool AltFireWeapon() override;
    virtual void LevelUpWeapon() override;
    virtual void StartReload() override;
    
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    // Ammo regeneration timer
    FTimerHandle AmmoRegenerationTimerHandle;
    
    // Alt fire cooldown timer
    FTimerHandle AltFireCooldownTimerHandle;
    
    // Projectile class for evolved weapon
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|R&C")
    TSubclassOf<class ARangedWeaponProjectile> EvolvedProjectileClass;
    
    // Handle ammo regeneration
    UFUNCTION()
    void RegenerateAmmo();
    
    // Complete alt fire cooldown
    UFUNCTION()
    void CompleteAltFireCooldown();
    
    // Apply upgrade effects
    void ApplyUpgradeEffects(const FWeaponUpgrade& Upgrade);
    
    // Get current upgrade level for a specific upgrade type
    int32 GetUpgradeLevel(EUpgradeType UpgradeType) const;
};