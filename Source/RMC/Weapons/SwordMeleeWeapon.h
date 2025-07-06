// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeWeaponBase.h"
#include "WeaponEnums.h"
#include "SwordMeleeWeapon.generated.h"

/**
 * Sword Melee Weapon class inspired by Devil May Cry's Rebellion/Red Queen.
 * Features balanced speed and power with an Exceed system for power boosts.
 */
UCLASS(Blueprintable)
class RMC_API USwordMeleeWeapon : public UMeleeWeaponBase
{
    GENERATED_BODY()
    
public:
    USwordMeleeWeapon();
    
    // DMC-specific properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    bool bHasExceedSystem;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    int32 MaxExceedLevel;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|DMC")
    int32 CurrentExceedLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float ExceedDamageMultiplier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float ExceedDuration;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float PerfectExceedWindow;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    UNiagaraSystem* ExceedEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    USoundBase* ExceedSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    UNiagaraSystem* MaxActEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    USoundBase* MaxActSound;
    
    // Aerial combat properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    bool bHasAerialRave;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float AerialDamageMultiplier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float AerialStyleMultiplier;
    
    // Guard properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    bool bCanParry;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float ParryWindow;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|DMC")
    float ParryDamageMultiplier;
    
    // DMC-style functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual bool AttemptExceed();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual bool AttemptMaxAct();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void ActivateExceed(int32 Levels = 1);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void ConsumeExceed();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual bool AttemptParry();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void PerformHighTime();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void PerformSplitSecond();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|DMC")
    virtual void PerformRoundTrip();
    
    // Override base functions with DMC-specific behavior
    virtual bool PerformAttack(EAttackInput AttackType) override;
    virtual bool ContinueCombo(EAttackInput AttackType) override;
    virtual void ChargeAttack(float ChargeAmount) override;
    virtual void ReleaseChargeAttack() override;
    virtual bool AttemptGuard() override;
    virtual FMeleeHitData ApplyDamage(AActor* Target, const FHitResult& Hit) override;
    
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExceedActivated, int32, ExceedLevel);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|DMC|Events")
    FOnExceedActivated OnExceedActivated;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMaxActPerformed);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|DMC|Events")
    FOnMaxActPerformed OnMaxActPerformed;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExceedDepleted);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|DMC|Events")
    FOnExceedDepleted OnExceedDepleted;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParrySuccessful);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|DMC|Events")
    FOnParrySuccessful OnParrySuccessful;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    
    // Exceed system timers
    FTimerHandle ExceedTimerHandle;
    FTimerHandle ParryWindowTimerHandle;
    
    // Exceed state tracking
    bool bIsExceedActive;
    float LastExceedAttemptTime;
    bool bInParryWindow;
    
    // Callbacks
    UFUNCTION()
    void OnExceedEnd();
    
    UFUNCTION()
    void OnParryWindowEnd();
    
    // Helper functions
    void PlayExceedEffect();
    void PlayMaxActEffect();
    float CalculateExceedDamageMultiplier() const;
    bool IsInAir() const;
};