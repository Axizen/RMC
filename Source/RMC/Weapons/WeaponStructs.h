// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponEnums.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "NiagaraSystem.h"
#include "WeaponStructs.generated.h"

// Struct for weapon upgrade grid nodes (Ratchet & Clank style)
USTRUCT(BlueprintType)
struct FWeaponUpgrade
{
	GENERATED_BODY()

	// Name of the upgrade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	FText UpgradeName;

	// Description of what the upgrade does
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	FText UpgradeDescription;

	// Cost in Raritanium points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	int32 Cost;

	// Whether this upgrade has been purchased
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	bool bPurchased;

	// Stat modifications
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	float DamageModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	float RangeModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	float FireRateModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	float AmmoCapacityModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	float ReloadTimeModifier;

	// Special effect unlocks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	bool bUnlocksElementalEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	EElementalType ElementalType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	bool bUnlocksProjectileTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	EProjectileTransform ProjectileTransform;

	// Grid position for UI display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	int32 GridX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	int32 GridY;

	// Prerequisite upgrades that must be purchased first (indices in the upgrade array)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Upgrade")
	TArray<int32> Prerequisites;

	FWeaponUpgrade()
		: UpgradeName(FText::FromString("Upgrade"))
		, UpgradeDescription(FText::FromString("Improves weapon performance"))
		, Cost(1)
		, bPurchased(false)
		, DamageModifier(0.0f)
		, RangeModifier(0.0f)
		, FireRateModifier(0.0f)
		, AmmoCapacityModifier(0.0f)
		, ReloadTimeModifier(0.0f)
		, bUnlocksElementalEffect(false)
		, ElementalType(EElementalType::None)
		, bUnlocksProjectileTransform(false)
		, ProjectileTransform(EProjectileTransform::None)
		, GridX(0)
		, GridY(0)
	{
	}
};

// Struct for weapon evolution data (Ratchet & Clank style)
USTRUCT(BlueprintType)
struct FWeaponEvolution
{
	GENERATED_BODY()

	// Name of the evolved weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	FText EvolvedName;

	// Description of the evolved weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	FText EvolvedDescription;

	// Level required to evolve
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	int32 RequiredLevel;

	// Whether the weapon has evolved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	bool bHasEvolved;

	// Base stat modifications when evolved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	float DamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	float RangeMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	float FireRateMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	float AmmoCapacityMultiplier;

	// New mesh/effects for the evolved weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	UStaticMesh* EvolvedMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	UMaterialInterface* EvolvedMaterial;

	// New projectile class for the evolved weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Evolution")
	TSubclassOf<AActor> EvolvedProjectileClass;

	FWeaponEvolution()
		: EvolvedName(FText::FromString("Evolved Weapon"))
		, EvolvedDescription(FText::FromString("An evolved version with enhanced capabilities"))
		, RequiredLevel(5)
		, bHasEvolved(false)
		, DamageMultiplier(1.5f)
		, RangeMultiplier(1.2f)
		, FireRateMultiplier(1.2f)
		, AmmoCapacityMultiplier(1.5f)
		, EvolvedMesh(nullptr)
		, EvolvedMaterial(nullptr)
		, EvolvedProjectileClass(nullptr)
	{
	}
};

// Struct for projectile transformation modifiers
USTRUCT(BlueprintType)
struct FTransformModifier
{
	GENERATED_BODY()

	// Type of transformation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	EProjectileTransform TransformType;

	// Name of the transformation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	FText TransformName;

	// Description of what the transformation does
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	FText TransformDescription;

	// Damage modifier when this transform is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	float DamageModifier;

	// Speed modifier when this transform is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	float SpeedModifier;

	// For split projectiles: number of projectiles to split into
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	int32 SplitCount;

	// For homing projectiles: turn rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	float HomingStrength;

	// For explosive projectiles: explosion radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	float ExplosionRadius;

	// For piercing projectiles: number of targets that can be pierced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	int32 PierceCount;

	// Sound for the transformed projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
	USoundBase* TransformSound;

	FTransformModifier()
		: TransformType(EProjectileTransform::None)
		, TransformName(FText::FromString("Standard"))
		, TransformDescription(FText::FromString("Standard projectile behavior"))
		, DamageModifier(1.0f)
		, SpeedModifier(1.0f)
		, SplitCount(3)
		, HomingStrength(5.0f)
		, ExplosionRadius(200.0f)
		, PierceCount(3)
		, TransformSound(nullptr)
	{
	}
};