// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponEnums.h"
#include "MeleeWeaponBase.h"
#include "WeaponManagerComponent.h"
#include "../MomentumComponent.h"
#include "MeleeWeaponManagerComponent.generated.h"

class UMeleeWeaponBase;
class UWeaponManagerComponent;
class UMomentumComponent;
class UStyleComponent;
class URiftComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UMeleeWeaponManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UMeleeWeaponManagerComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Melee weapon inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapons")
    TArray<UMeleeWeaponBase*> MeleeWeaponInventory;
    
    // Currently equipped melee weapon
    UPROPERTY(BlueprintReadOnly, Category = "Melee Weapons")
    UMeleeWeaponBase* CurrentMeleeWeapon;
    
    // Current melee weapon index
    UPROPERTY(BlueprintReadOnly, Category = "Melee Weapons")
    int32 CurrentMeleeWeaponIndex;
    
    // Maximum number of melee weapons that can be carried
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Weapons")
    int32 MaxMeleeWeapons;
    
    // Combat style (DMC-inspired)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Style")
    ECombatStyle CurrentCombatStyle;
    
    // Style component reference
    UPROPERTY()
    UStyleComponent* StyleComponent;
    
    // Momentum component reference
    UPROPERTY()
    UMomentumComponent* MomentumComponent;
    
    // Rift component reference
    UPROPERTY()
    URiftComponent* RiftComponent;
    
    // Ranged weapon manager reference
    UPROPERTY()
    UWeaponManagerComponent* WeaponManagerComponent;
    
    // Whether currently in melee stance (vs ranged stance)
    UPROPERTY(BlueprintReadOnly, Category = "Combat Stance")
    bool bInMeleeStance;
    
    // Functions
    UFUNCTION(BlueprintCallable, Category = "Melee Weapons")
    void SwitchToMeleeWeapon(int32 WeaponIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Melee Weapons")
    void NextMeleeWeapon();
    
    UFUNCTION(BlueprintCallable, Category = "Melee Weapons")
    void PreviousMeleeWeapon();
    
    UFUNCTION(BlueprintCallable, Category = "Melee Weapons")
    bool AddMeleeWeaponToInventory(TSubclassOf<UMeleeWeaponBase> WeaponClass);
    
    UFUNCTION(BlueprintCallable, Category = "Melee Weapons")
    bool RemoveMeleeWeaponFromInventory(int32 WeaponIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Melee Weapons")
    bool HasMeleeWeapon(TSubclassOf<UMeleeWeaponBase> WeaponClass) const;
    
    UFUNCTION(BlueprintPure, Category = "Melee Weapons")
    int32 GetMeleeWeaponCount() const { return MeleeWeaponInventory.Num(); }
    
    // Combat style functions
    UFUNCTION(BlueprintCallable, Category = "Combat Style")
    void SwitchCombatStyle(ECombatStyle NewStyle);
    
    UFUNCTION(BlueprintPure, Category = "Combat Style")
    bool CanUseCombatStyle(ECombatStyle Style) const;
    
    // Combat stance functions
    UFUNCTION(BlueprintCallable, Category = "Combat Stance")
    void SwitchToMeleeStance();
    
    UFUNCTION(BlueprintCallable, Category = "Combat Stance")
    void SwitchToRangedStance();
    
    UFUNCTION(BlueprintCallable, Category = "Combat Stance")
    void ToggleCombatStance();
    
    // Attack input handling
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformLightAttack();
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformHeavyAttack();
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformDirectionalAttack(EDirection Direction);
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformSpecialAttack();
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool AttemptGuard();
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndGuard();
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool AttemptJumpCancel();
    
    // Charge attack functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StartChargeAttack();
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ReleaseChargeAttack();
    
    // Perfect timing (Exceed) system
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool AttemptExceedTiming();
    
    // Rift integration
    UFUNCTION()
    void OnRiftBegin();
    
    UFUNCTION()
    void OnRiftEnd();
    
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeleeWeaponSwitched, UMeleeWeaponBase*, NewWeapon, UMeleeWeaponBase*, PreviousWeapon);
    UPROPERTY(BlueprintAssignable, Category = "Melee Weapons|Events")
    FOnMeleeWeaponSwitched OnMeleeWeaponSwitched;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeWeaponAdded, UMeleeWeaponBase*, NewWeapon);
    UPROPERTY(BlueprintAssignable, Category = "Melee Weapons|Events")
    FOnMeleeWeaponAdded OnMeleeWeaponAdded;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeWeaponRemoved, UMeleeWeaponBase*, RemovedWeapon);
    UPROPERTY(BlueprintAssignable, Category = "Melee Weapons|Events")
    FOnMeleeWeaponRemoved OnMeleeWeaponRemoved;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStyleChanged, ECombatStyle, NewStyle);
    UPROPERTY(BlueprintAssignable, Category = "Combat Style|Events")
    FOnCombatStyleChanged OnCombatStyleChanged;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStanceChanged, bool, bIsInMeleeStance);
    UPROPERTY(BlueprintAssignable, Category = "Combat Stance|Events")
    FOnCombatStanceChanged OnCombatStanceChanged;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    
    // Helper functions to get owner components
    UStyleComponent* GetOwnerStyleComponent() const;
    UMomentumComponent* GetOwnerMomentumComponent() const;
    URiftComponent* GetOwnerRiftComponent() const;
    UWeaponManagerComponent* GetOwnerWeaponManagerComponent() const;
    
    // Input buffer for combo detection
    struct FBufferedInput
    {
        EAttackInput InputType;
        float TimeStamp;
        
        FBufferedInput() : InputType(EAttackInput::None), TimeStamp(0.0f) {}
        FBufferedInput(EAttackInput InType, float InTime) : InputType(InType), TimeStamp(InTime) {}
    };
    
    TArray<FBufferedInput> InputBuffer;
    float InputBufferTimeWindow;
    
    // Process buffered inputs
    bool ProcessBufferedInput(EAttackInput NewInput);
    
    // Combat style unlocks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Style")
    TArray<ECombatStyle> UnlockedStyles;
    
    // Style-specific move unlocks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Style")
    TMap<ECombatStyle, int32> StyleMasteryLevels;
};