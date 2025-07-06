// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangedWeaponBase.h"
#include "WeaponEnums.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrecisionRangedWeapon.generated.h"

class AActor;

/**
 * Precision Ranged Weapon class inspired by Devil May Cry's gunplay mechanics.
 * Focuses on style, combo potential, and technical execution rather than raw damage.
 */
UCLASS(Blueprintable)
class RMC_API UPrecisionRangedWeapon : public URangedWeaponBase
{
    GENERATED_BODY()
    
public:
    UPrecisionRangedWeapon();
    
    // DMC-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    bool bAutoTargeting;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float StyleMultiplier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    bool bCanJuggleEnemies;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float JuggleForce;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    TArray<FName> ComboCompatibleMoves;
    
    // Rapid fire properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    bool bCanRapidFire;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float RapidFireRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    int32 RapidFireBurstCount;
    
    // Charge shot properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    bool bHasChargeShot;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float MaxChargeTime;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float ChargeMultiplier;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|DMC")
    float CurrentChargeTime;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|DMC")
    bool bIsCharging;
    
    // Socket name for weapon effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    FName SocketName;
    
    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    UNiagaraSystem* ChargeEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    UNiagaraSystem* SpecialEffect;
    
    // Effect state
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|DMC")
    bool bIsEffectPlaying;
    
    // DMC-style functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void PerformGunslinger(EDirection InputDirection);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void CancelAttackWithGun();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void JuggleTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void StartRapidFire();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void StopRapidFire();
    
    // Override base functions with DMC-specific behavior
    virtual bool FireWeapon() override;
    virtual bool AltFireWeapon() override;
    virtual void ChargeWeapon(float ChargeAmount) override;
    virtual void ReleaseCharge() override;
    virtual void OnRiftBegin() override;
    
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    // Rapid fire timer
    FTimerHandle RapidFireTimerHandle;
    
    // Cooldown timer
    FTimerHandle CooldownTimerHandle;
    
    // Rapid fire implementation
    UFUNCTION()
    void HandleRapidFire();
    
    // Find best target for auto-targeting
    AActor* FindBestTarget() const;
    
    // Calculate damage based on charge level
    float CalculateChargeDamage() const;
    
    // Helper function to play effects
    UFUNCTION()
    void PlayEffect(UNiagaraSystem* Effect, FVector Location, FRotator Rotation);
    
    // Cooldown complete callback
    UFUNCTION()
    void OnCooldownComplete();
};