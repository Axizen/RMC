// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponEnums.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Firing UMETA(DisplayName = "Firing"),
	Reloading UMETA(DisplayName = "Reloading"),
	Charging UMETA(DisplayName = "Charging")
};

UENUM(BlueprintType)
enum class EStyleRank : uint8
{
	D UMETA(DisplayName = "D"),
	C UMETA(DisplayName = "C"),
	B UMETA(DisplayName = "B"),
	A UMETA(DisplayName = "A"),
	S UMETA(DisplayName = "S"),
	SS UMETA(DisplayName = "SS"),
	SSS UMETA(DisplayName = "SSS")
};

UENUM(BlueprintType)
enum class EDirection : uint8
{
	None UMETA(DisplayName = "None"),
	Forward UMETA(DisplayName = "Forward"),
	Backward UMETA(DisplayName = "Backward"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	Up UMETA(DisplayName = "Up"),
	Down UMETA(DisplayName = "Down")
};

UENUM(BlueprintType)
enum class EElementalType : uint8
{
	None UMETA(DisplayName = "None"),
	Fire UMETA(DisplayName = "Fire"),
	Ice UMETA(DisplayName = "Ice"),
	Electric UMETA(DisplayName = "Electric"),
	Poison UMETA(DisplayName = "Poison")
};

UENUM(BlueprintType)
enum class EProjectileTransform : uint8
{
	None UMETA(DisplayName = "None"),
	Split UMETA(DisplayName = "Split"),
	Homing UMETA(DisplayName = "Homing"),
	Explosive UMETA(DisplayName = "Explosive"),
	Piercing UMETA(DisplayName = "Piercing")
};

UENUM(BlueprintType)
enum class EAltFireMode : uint8
{
	None UMETA(DisplayName = "None"),
	Charge UMETA(DisplayName = "Charge"),
	Burst UMETA(DisplayName = "Burst"),
	Spread UMETA(DisplayName = "Spread"),
	Elemental UMETA(DisplayName = "Elemental")
};

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	Damage UMETA(DisplayName = "Damage"),
	Range UMETA(DisplayName = "Range"),
	FireRate UMETA(DisplayName = "Fire Rate"),
	AmmoCapacity UMETA(DisplayName = "Ammo Capacity"),
	ReloadTime UMETA(DisplayName = "Reload Time"),
	Elemental UMETA(DisplayName = "Elemental"),
	Projectile UMETA(DisplayName = "Projectile")
};

// Melee weapon types for DMC-inspired combat system
UENUM(BlueprintType)
enum class EMeleeWeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	Sword UMETA(DisplayName = "Sword"),
	DualBlades UMETA(DisplayName = "Dual Blades"),
	Gauntlets UMETA(DisplayName = "Gauntlets"),
	HeavyWeapon UMETA(DisplayName = "Heavy Weapon"),
	SpecialWeapon UMETA(DisplayName = "Special Weapon")
};

// Attack input types for melee combat
UENUM(BlueprintType)
enum class EAttackInput : uint8
{
	None UMETA(DisplayName = "None"),
	Light UMETA(DisplayName = "Light Attack"),
	Heavy UMETA(DisplayName = "Heavy Attack"),
	Forward UMETA(DisplayName = "Forward Attack"),
	Back UMETA(DisplayName = "Back Attack"),
	Up UMETA(DisplayName = "Up Attack"),
	Down UMETA(DisplayName = "Down Attack"),
	Special UMETA(DisplayName = "Special Attack"),
	Exceed UMETA(DisplayName = "Exceed")
};

// Melee weapon state
UENUM(BlueprintType)
enum class EMeleeWeaponState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Attacking UMETA(DisplayName = "Attacking"),
	Charging UMETA(DisplayName = "Charging"),
	Guarding UMETA(DisplayName = "Guarding")
};

// Combat style types (DMC-inspired)
UENUM(BlueprintType)
enum class ECombatStyle : uint8
{
	None UMETA(DisplayName = "None"),
	Swordmaster UMETA(DisplayName = "Swordmaster"),
	Gunslinger UMETA(DisplayName = "Gunslinger"),
	Trickster UMETA(DisplayName = "Trickster"),
	RoyalGuard UMETA(DisplayName = "Royal Guard")
};