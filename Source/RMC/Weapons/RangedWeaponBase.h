// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponEnums.h"
#include "WeaponStructs.h"
#include "NiagaraSystem.h"
#include "../MomentumComponent.h"
#include "RangedWeaponBase.generated.h"

class URiftComponent;
class UMomentumComponent;

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API URangedWeaponBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URangedWeaponBase();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Core weapon properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float BaseDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float FireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	int32 MaxAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	int32 CurrentAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float ReloadTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	int32 AmmoPerShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float ChargeTime;
	
	// Weapon state
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	EWeaponState CurrentState;
	
	// Current charge level (0.0 to 1.0)
	UPROPERTY(BlueprintReadOnly, Category = "Weapon|State")
	float CurrentCharge;
	
	// Style system integration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Style")
	float StylePointsPerHit;
	
	// Rift system integration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Rift")
	float MomentumGainPerHit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Rift")
	bool bCanFireDuringRift;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Rift")
	float RiftDamageMultiplier;
	
	// Weapon level system (R&C inspired)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Progression")
	int32 WeaponLevel;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Progression")
	float WeaponXP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Progression")
	TArray<float> LevelXPThresholds;

	// Visual and audio effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	UNiagaraSystem* MuzzleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Effects")
	USoundBase* EmptySound;

	// Projectile class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	TSubclassOf<AActor> ProjectileClass;

	// Weapon display info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
	FText WeaponDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|UI")
	UTexture2D* WeaponIcon;

	// Core functions
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool FireWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool AltFireWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void FinishReload();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void ChargeWeapon(float ChargeAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void ReleaseCharge();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void SpawnProjectile(FVector Location, FRotator Rotation, float DamageOverride = -1.0f);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	virtual bool CanFire() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void ConsumeAmmo(int32 Amount);
	
	// Rift integration functions
	UFUNCTION(BlueprintCallable, Category = "Weapon|Rift")
	virtual void OnRiftBegin();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon|Rift")
	virtual void OnRiftEnd();
	
	UFUNCTION(BlueprintCallable, Category = "Weapon|Rift")
	virtual void FireDuringRift();
	
	// Progression functions
	UFUNCTION(BlueprintCallable, Category = "Weapon|Progression")
	virtual void AddWeaponXP(float XPAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon|Progression")
	virtual void LevelUpWeapon();

	UFUNCTION(BlueprintPure, Category = "Weapon|Progression")
	virtual float GetXPToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Progression")
	virtual float GetLevelProgress() const;
	
	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponFired, URangedWeaponBase*, Weapon);
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponFired OnWeaponFired;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponLevelUp, int32, NewLevel);
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponLevelUp OnWeaponLevelUp;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponReloaded);
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponReloaded OnWeaponReloaded;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponEmpty);
	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponEmpty OnWeaponEmpty;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Reference to the owner's rift component
	UPROPERTY()
	URiftComponent* OwnerRiftComponent;

	// Reference to the owner's momentum component
	UPROPERTY()
	UMomentumComponent* OwnerMomentumComponent;



	// Timer handles
	FTimerHandle ReloadTimerHandle;
	FTimerHandle FireRateTimerHandle;

	// Helper function to get the owner's rift component
	URiftComponent* GetOwnerRiftComponent() const;

	// Helper function to get the owner's momentum component
	UMomentumComponent* GetOwnerMomentumComponent() const;

	// Helper function to play weapon effects
	virtual void PlayWeaponEffects(UNiagaraSystem* Effect, USoundBase* Sound);
};