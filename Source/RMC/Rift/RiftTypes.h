// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RiftTypes.generated.h"

UENUM(BlueprintType)
enum class ERiftState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Tethering UMETA(DisplayName = "Tethering"),
	Dodging UMETA(DisplayName = "Dodging"),
	Cooldown UMETA(DisplayName = "Cooldown")
};

USTRUCT(BlueprintType)
struct FRiftCapabilities
{
	GENERATED_BODY()

	// Maximum distance for rift tethering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	float MaxRiftDistance = 500.0f;

	// Maximum number of chain rifts allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	int32 MaxChainCount = 1;

	// Duration of phantom dodge invulnerability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	float PhantomDodgeDuration = 0.3f;

	// Distance of phantom dodge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	float PhantomDodgeDistance = 300.0f;

	// Whether aerial rift reset is available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	bool bCanAerialReset = false;

	// Whether counter rift is available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	bool bCanCounterRift = false;
};