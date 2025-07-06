// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponEnums.h"
#include "../MomentumComponent.h"
#include "StyleComponent.h"
#include "NiagaraSystem.h"
#include "MeleeWeaponBase.generated.h"

class UAnimMontage;
class USkeletalMeshComponent;
class UAnimInstance;
class UMomentumComponent;
class UStyleComponent;

// Struct for melee combo node data
USTRUCT(BlueprintType)
struct FMeleeComboNode
{
    GENERATED_BODY()
    
    // Animation to play for this combo node
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    UAnimMontage* ComboAnimation;
    
    // Name of this move for style system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FText MoveName;
    
    // Damage for this combo hit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float Damage;
    
    // Style points awarded
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float StylePoints;
    
    // Momentum gained
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    float MomentumGain;
    
    // Next combo nodes (branching paths)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    TMap<EAttackInput, int32> NextComboIndices;
    
    // Whether this attack can launch enemies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    bool bCanLaunchEnemy;
    
    // Force applied to enemies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    FVector KnockbackForce;
    
    // Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UNiagaraSystem* ImpactEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    USoundBase* ImpactSound;
    
    // Whether this move can be jump-canceled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bCanJumpCancel;
    
    // Whether this move has perfect timing (just-frame) window
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bHasPerfectTimingWindow;
    
    // Perfect timing window in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    float PerfectTimingWindow;
    
    // Damage multiplier for perfect timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    float PerfectTimingDamageMultiplier;
    
    FMeleeComboNode()
        : ComboAnimation(nullptr)
        , MoveName(FText::FromString("Attack"))
        , Damage(10.0f)
        , StylePoints(100.0f)
        , MomentumGain(5.0f)
        , bCanLaunchEnemy(false)
        , KnockbackForce(FVector(100.0f, 0.0f, 0.0f))
        , ImpactEffect(nullptr)
        , ImpactSound(nullptr)
        , bCanJumpCancel(false)
        , bHasPerfectTimingWindow(false)
        , PerfectTimingWindow(0.1f)
        , PerfectTimingDamageMultiplier(1.5f)
    {
    }
};

// Struct for hit detection data
USTRUCT(BlueprintType)
struct FMeleeHitData
{
    GENERATED_BODY()
    
    // Actor that was hit
    UPROPERTY(BlueprintReadOnly)
    AActor* HitActor;
    
    // Hit result
    UPROPERTY(BlueprintReadOnly)
    FHitResult HitResult;
    
    // Damage applied
    UPROPERTY(BlueprintReadOnly)
    float DamageApplied;
    
    // Whether this was a critical hit
    UPROPERTY(BlueprintReadOnly)
    bool bWasCriticalHit;
    
    // Whether this was a perfect timing hit
    UPROPERTY(BlueprintReadOnly)
    bool bWasPerfectTiming;
    
    FMeleeHitData()
        : HitActor(nullptr)
        , DamageApplied(0.0f)
        , bWasCriticalHit(false)
        , bWasPerfectTiming(false)
    {
    }
    
    FMeleeHitData(AActor* InHitActor, const FHitResult& InHitResult, float InDamageApplied, bool InWasCriticalHit, bool InWasPerfectTiming)
        : HitActor(InHitActor)
        , HitResult(InHitResult)
        , DamageApplied(InDamageApplied)
        , bWasCriticalHit(InWasCriticalHit)
        , bWasPerfectTiming(InWasPerfectTiming)
    {
    }
};

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMeleeWeaponBase : public UActorComponent
{
    GENERATED_BODY()
    
public:
    // Sets default values for this component's properties
    UMeleeWeaponBase();
    
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Core properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float BaseDamage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float AttackSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    float WeaponReach;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    EMeleeWeaponType WeaponType;
    
    // Current weapon state
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
    EMeleeWeaponState CurrentState;
    
    // Combo system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo")
    TArray<FMeleeComboNode> ComboNodes;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Combo")
    int32 CurrentComboIndex;
    
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Combo")
    int32 ComboCount;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Combo")
    float ComboTimeWindow;
    
    // Style integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Style")
    float StylePointsPerHit;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Style")
    float StyleMultiplier;
    
    // Momentum integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Momentum")
    float MomentumGainPerHit;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Momentum")
    float MomentumDamageMultiplier;
    
    // Visual elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Visual")
    UStaticMesh* WeaponMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Visual")
    UMaterialInterface* WeaponMaterial;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Visual")
    UNiagaraSystem* TrailEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Visual")
    FName AttachSocketName;
    
    // Weapon display info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
    FText WeaponName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
    FText WeaponDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
    UTexture2D* WeaponIcon;
    
    // Advanced mechanics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Advanced")
    bool bCanJumpCancel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Advanced")
    bool bCanGuard;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Advanced")
    float GuardDamageReduction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Advanced")
    bool bHasAerialCombos;
    
    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual bool PerformAttack(EAttackInput AttackType);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual bool ContinueCombo(EAttackInput AttackType);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual void ResetCombo();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual void ChargeAttack(float ChargeAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual void ReleaseChargeAttack();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual void PerformSpecialAttack(EAttackInput AttackType, EDirection Direction);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual bool AttemptGuard();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual void EndGuard();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual bool AttemptJumpCancel();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual bool IsInCombo() const;
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual bool CanAttack() const;
    
    // Hit detection
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual void CheckHit();
    
    UFUNCTION(BlueprintCallable, Category = "Weapon|Combat")
    virtual FMeleeHitData ApplyDamage(AActor* Target, const FHitResult& Hit);
    
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboProgressed, int32, NewComboCount);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnComboProgressed OnComboProgressed;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboReset);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnComboReset OnComboReset;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyHit, const FMeleeHitData&, HitData);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnEnemyHit OnEnemyHit;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerfectTimingPerformed, int32, ComboNodeIndex);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnPerfectTimingPerformed OnPerfectTimingPerformed;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuardBegin);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnGuardBegin OnGuardBegin;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGuardEnd);
    UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
    FOnGuardEnd OnGuardEnd;
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    
    // References to other components
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;
    
    UPROPERTY()
    UAnimInstance* OwnerAnimInstance;
    
    UPROPERTY()
    UStyleComponent* StyleComponent;
    
    UPROPERTY()
    UMomentumComponent* MomentumComponent;
    
    // Combo system helpers
    FTimerHandle ComboTimerHandle;
    
    UFUNCTION()
    void OnComboTimeout();
    
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    
    // State tracking
    bool bIsAttacking;
    bool bCanContinueCombo;
    bool bIsCharging;
    float CurrentChargeAmount;
    bool bIsGuarding;
    
    // Last hit tracking for jump cancels
    bool bLastHitSuccessful;
    
    // Perfect timing tracking
    float LastAttackInputTime;
    
    // Helper functions
    UStyleComponent* GetOwnerStyleComponent() const;
    UMomentumComponent* GetOwnerMomentumComponent() const;
    void PlayImpactEffects(const FHitResult& Hit);
    bool CheckPerfectTiming(float InputTime, float IdealTime, float Window) const;
};