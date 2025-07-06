// Fill out your copyright notice in the Description page of Project Settings.

#include "RangedWeaponBase.h"
#include "../Rift/RiftComponent.h"
#include "../MomentumComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RangedWeaponProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
URangedWeaponBase::URangedWeaponBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default weapon stats
	BaseDamage = 20.0f;
	FireRate = 0.2f;
	MaxAmmo = 30;
	CurrentAmmo = MaxAmmo;
	ReloadTime = 1.5f;
	Range = 1000.0f;
	AmmoPerShot = 1;
	ChargeTime = 1.0f;

	// Default state
	CurrentState = EWeaponState::Idle;
	CurrentCharge = 0.0f;

	// Style and rift integration
	StylePointsPerHit = 10.0f;
	MomentumGainPerHit = 5.0f;
	bCanFireDuringRift = false;
	RiftDamageMultiplier = 1.5f;

	// Progression defaults
	WeaponLevel = 1;
	WeaponXP = 0.0f;
	LevelXPThresholds = { 100.0f, 250.0f, 500.0f, 1000.0f, 2000.0f };

	// Default display info
	WeaponName = FText::FromString("Ranged Weapon");
	WeaponDescription = FText::FromString("A basic ranged weapon");
}

// Called when the game starts
void URangedWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// Get reference to owner's rift component
	OwnerRiftComponent = GetOwnerRiftComponent();

	// Get reference to owner's momentum component
	OwnerMomentumComponent = GetOwnerMomentumComponent();

	// If we have a rift component, bind to its events
	if (OwnerRiftComponent)
	{
		OwnerRiftComponent->OnRiftBegin.AddDynamic(this, &URangedWeaponBase::OnRiftBegin);
		OwnerRiftComponent->OnRiftEnd.AddDynamic(this, &URangedWeaponBase::OnRiftEnd);
	}
}

// Called every frame
void URangedWeaponBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Handle charging state
	if (CurrentState == EWeaponState::Charging)
	{
		// Increment charge based on delta time
		CurrentCharge = FMath::Min(CurrentCharge + (DeltaTime / ChargeTime), 1.0f);
	}
}

bool URangedWeaponBase::FireWeapon()
{
	// Check if we can fire
	if (!CanFire())
	{
		// Play empty sound if we're out of ammo
		if (CurrentAmmo <= 0 && EmptySound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), EmptySound);
			OnWeaponEmpty.Broadcast();
		}
		return false;
	}

	// Set state to firing
	CurrentState = EWeaponState::Firing;

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

	// Calculate muzzle location (can be improved with actual socket location from a weapon mesh)
	FVector MuzzleLocation = CameraLocation + CameraRotation.Vector() * 100.0f;

	// Calculate damage with momentum modifier
	float ModifiedDamage = BaseDamage;
	if (OwnerMomentumComponent)
	{
		// Apply damage bonus based on momentum tier
		int32 MomentumTier = OwnerMomentumComponent->GetMomentumTier();
		float MomentumDamageMultiplier = 1.0f + (MomentumTier * 0.1f); // 10% damage increase per tier
		ModifiedDamage *= MomentumDamageMultiplier;
	}

	// Spawn projectile
	SpawnProjectile(MuzzleLocation, CameraRotation, ModifiedDamage);

	// Play effects
	PlayWeaponEffects(MuzzleEffect, FireSound);

	// Consume ammo
	ConsumeAmmo(AmmoPerShot);

	// Broadcast fired event
	OnWeaponFired.Broadcast(this);

	// Set timer to return to idle state based on fire rate
	GetWorld()->GetTimerManager().SetTimer(
		FireRateTimerHandle,
		[this]()
		{
			CurrentState = EWeaponState::Idle;
		},
		FireRate,
		false
	);

	return true;
}

bool URangedWeaponBase::AltFireWeapon()
{
	// Base implementation just starts charging
	if (CurrentState == EWeaponState::Idle && CurrentAmmo >= AmmoPerShot)
	{
		CurrentState = EWeaponState::Charging;
		CurrentCharge = 0.0f;
		return true;
	}
	return false;
}

void URangedWeaponBase::StartReload()
{
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

	// Set timer for reload completion
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&URangedWeaponBase::FinishReload,
		ReloadTime,
		false
	);
}

void URangedWeaponBase::FinishReload()
{
	// Refill ammo
	CurrentAmmo = MaxAmmo;

	// Return to idle state
	CurrentState = EWeaponState::Idle;

	// Broadcast reloaded event
	OnWeaponReloaded.Broadcast();
}

void URangedWeaponBase::ChargeWeapon(float ChargeAmount)
{
	// This is called externally to manually charge the weapon
	if (CurrentState == EWeaponState::Charging)
	{
		CurrentCharge = FMath::Clamp(CurrentCharge + ChargeAmount, 0.0f, 1.0f);
	}
}

void URangedWeaponBase::ReleaseCharge()
{
	// Only process if we're in charging state
	if (CurrentState != EWeaponState::Charging)
	{
		return;
	}

	// Check if we have enough ammo
	if (CurrentAmmo < AmmoPerShot)
	{
		// Play empty sound
		if (EmptySound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), EmptySound);
			OnWeaponEmpty.Broadcast();
		}

		// Return to idle state
		CurrentState = EWeaponState::Idle;
		CurrentCharge = 0.0f;
		return;
	}

	// Get owner character for spawn location
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		CurrentState = EWeaponState::Idle;
		CurrentCharge = 0.0f;
		return;
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

	// Calculate damage based on charge level
	float ChargeDamageMultiplier = 1.0f + (CurrentCharge * 2.0f); // Up to 3x damage at full charge
	float ChargeDamage = BaseDamage * ChargeDamageMultiplier;
	
	// Apply momentum bonus if available
	if (OwnerMomentumComponent)
	{
		// Get momentum tier and ratio
		int32 MomentumTier = OwnerMomentumComponent->GetMomentumTier();
		float MomentumRatio = OwnerMomentumComponent->GetMomentumRatio();
		
		// Apply momentum-based damage bonus
		float MomentumDamageMultiplier = 1.0f + (MomentumTier * 0.15f); // 15% damage increase per tier
		ChargeDamage *= MomentumDamageMultiplier;
		
		// Increase charge effect at high momentum
		if (MomentumRatio > 0.7f)
		{
			// Bonus charge effect at high momentum
			ChargeDamage *= 1.2f;
		}
	}

	// Spawn projectile with modified damage
	SpawnProjectile(MuzzleLocation, CameraRotation, ChargeDamage);

	// Play effects
	PlayWeaponEffects(MuzzleEffect, FireSound);

	// Consume ammo
	ConsumeAmmo(AmmoPerShot);

	// Broadcast fired event
	OnWeaponFired.Broadcast(this);

	// Reset charge and return to idle state
	CurrentCharge = 0.0f;
	CurrentState = EWeaponState::Idle;
}

void URangedWeaponBase::SpawnProjectile(FVector Location, FRotator Rotation, float DamageOverride)
{
	// Check if we have a valid projectile class
	if (!ProjectileClass || !GetWorld())
	{
		return;
	}

	// Setup spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the projectile
	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, Location, Rotation, SpawnParams);

	// Additional setup for the projectile
	ARangedWeaponProjectile* WeaponProjectile = Cast<ARangedWeaponProjectile>(Projectile);
	if (WeaponProjectile)
	{
		// Set damage based on override or base damage
		WeaponProjectile->Damage = (DamageOverride > 0.0f) ? DamageOverride : BaseDamage;
		
		// Set style points and momentum gain
		WeaponProjectile->StylePointsOnHit = StylePointsPerHit;
		WeaponProjectile->MomentumGainOnHit = MomentumGainPerHit;
		
		// Apply momentum-based effects if available
		if (OwnerMomentumComponent)
		{
			float MomentumRatio = OwnerMomentumComponent->GetMomentumRatio();
			
			// Increase projectile speed based on momentum
			if (WeaponProjectile->ProjectileMovement)
			{
				WeaponProjectile->ProjectileMovement->InitialSpeed *= (1.0f + (MomentumRatio * 0.3f));
				WeaponProjectile->ProjectileMovement->MaxSpeed *= (1.0f + (MomentumRatio * 0.3f));
			}
			
			// Increase style points based on momentum
			WeaponProjectile->StylePointsOnHit *= (1.0f + (MomentumRatio * 0.5f));
		}
	}
}

bool URangedWeaponBase::CanFire() const
{
	// Check if we're in a state that allows firing
	if (CurrentState != EWeaponState::Idle)
	{
		return false;
	}

	// Check if we have enough ammo
	if (CurrentAmmo < AmmoPerShot)
	{
		return false;
	}

	return true;
}

void URangedWeaponBase::ConsumeAmmo(int32 Amount)
{
	CurrentAmmo = FMath::Max(CurrentAmmo - Amount, 0);

	// Auto-reload when empty (can be disabled in derived classes)
	if (CurrentAmmo == 0)
	{
		OnWeaponEmpty.Broadcast();
		StartReload();
	}
}

void URangedWeaponBase::OnRiftBegin()
{
	// Base implementation - can be extended in derived classes
}

void URangedWeaponBase::OnRiftEnd()
{
	// Base implementation - can be extended in derived classes
}

void URangedWeaponBase::FireDuringRift()
{
	// Only allow if the weapon can fire during rifts
	if (!bCanFireDuringRift)
	{
		return;
	}

	// Special implementation for firing during rifts
	// This is a simplified version that just calls FireWeapon
	// Derived classes can implement more complex behavior
	FireWeapon();
}

void URangedWeaponBase::AddWeaponXP(float XPAmount)
{
	// Add XP to the weapon
	WeaponXP += XPAmount;

	// Check if we should level up
	if (WeaponLevel <= LevelXPThresholds.Num() && 
		WeaponXP >= LevelXPThresholds[WeaponLevel - 1])
	{
		LevelUpWeapon();
	}
}

void URangedWeaponBase::LevelUpWeapon()
{
	// Increment level
	WeaponLevel++;

	// Base implementation just increases damage
	BaseDamage *= 1.2f;

	// Broadcast level up event
	OnWeaponLevelUp.Broadcast(WeaponLevel);
}

float URangedWeaponBase::GetXPToNextLevel() const
{
	// If we're at max level, return 0
	if (WeaponLevel > LevelXPThresholds.Num())
	{
		return 0.0f;
	}

	// Return XP needed for next level
	return LevelXPThresholds[WeaponLevel - 1] - WeaponXP;
}

float URangedWeaponBase::GetLevelProgress() const
{
	// If we're at max level, return 1.0
	if (WeaponLevel > LevelXPThresholds.Num())
	{
		return 1.0f;
	}

	// Calculate progress as percentage to next level
	float CurrentLevelXP = (WeaponLevel > 1) ? LevelXPThresholds[WeaponLevel - 2] : 0.0f;
	float NextLevelXP = LevelXPThresholds[WeaponLevel - 1];
	float LevelXPRange = NextLevelXP - CurrentLevelXP;

	return (WeaponXP - CurrentLevelXP) / LevelXPRange;
}

URiftComponent* URangedWeaponBase::GetOwnerRiftComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<URiftComponent>();
}

UMomentumComponent* URangedWeaponBase::GetOwnerMomentumComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UMomentumComponent>();
}

void URangedWeaponBase::PlayWeaponEffects(UNiagaraSystem* Effect, USoundBase* Sound)
{
	// Play sound effect
	if (Sound && GetWorld())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}

	// Spawn visual effect
	if (Effect && GetWorld())
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (OwnerCharacter)
		{
			// In a real implementation, this would use the weapon mesh socket
			// For now, just spawn at character location + offset
			FVector Location = OwnerCharacter->GetActorLocation() + 
				(OwnerCharacter->GetActorForwardVector() * 100.0f) + 
				FVector(0, 0, 50.0f);

			// Use Niagara system for visual effects
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				Effect,
				Location,
				OwnerCharacter->GetActorRotation()
			);
			
			// Alternative: Use particle system if Niagara is not available
			// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleEffect, Location, OwnerCharacter->GetActorRotation());
		}
	}
}