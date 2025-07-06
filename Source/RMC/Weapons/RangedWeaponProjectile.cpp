// Fill out your copyright notice in the Description page of Project Settings.

#include "RangedWeaponProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../Rift/RiftAnchor.h"
#include "../MomentumComponent.h"

// Sets default values
ARangedWeaponProjectile::ARangedWeaponProjectile()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Create and set up collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName("Projectile");
	CollisionComponent->OnComponentHit.AddDynamic(this, &ARangedWeaponProjectile::OnHit);

	// Set as root component
	RootComponent = CollisionComponent;

	// Create and set up projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // No gravity by default

	// Create and set up effect component
	EffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectComponent"));
	EffectComponent->SetupAttachment(RootComponent);
	EffectComponent->SetAutoActivate(true);

	// Default properties
	Damage = 20.0f;
	bCanRiftTo = false;
	bCreateAnchorOnImpact = false;
	StylePointsOnHit = 10.0f;
	MomentumGainOnHit = 5.0f;
	LifeSpan = 5.0f;
	bExplodeOnImpact = false;
	ExplosionRadius = 200.0f;
	ElementalType = EElementalType::None;
	ElementalEffectChance = 0.25f;

	// Set default lifespan
	InitialLifeSpan = 5.0f;
}

// Called when the game starts or when spawned
void ARangedWeaponProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// Set lifespan from property
	SetLifeSpan(LifeSpan);
}

// Called every frame
void ARangedWeaponProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARangedWeaponProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Don't hit the owner
	if (OtherActor && OtherActor != GetOwner())
	{
		// Broadcast hit event
		OnProjectileHit.Broadcast(Hit);

		// Apply damage to the hit actor
		ApplyDamage(OtherActor, Hit);

		// Apply elemental effect
		if (ElementalType != EElementalType::None && FMath::FRand() <= ElementalEffectChance)
		{
			ApplyElementalEffect(OtherActor);
		}

		// Create a rift anchor if enabled
		if (bCreateAnchorOnImpact)
		{
			CreateRiftAnchor(Hit.Location);
		}

		// Add momentum to the owner if applicable
		if (GetOwner())
		{
			UMomentumComponent* OwnerMomentumComp = GetOwner()->FindComponentByClass<UMomentumComponent>();
			if (OwnerMomentumComp)
			{
				// Add momentum based on hit
				OwnerMomentumComp->AddMomentum(MomentumGainOnHit, FName("ProjectileHit"));
				
				// Add bonus momentum for headshots or critical hits
				if (Hit.BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase) ||
				    Hit.BoneName.ToString().Contains(TEXT("neck"), ESearchCase::IgnoreCase))
				{
					// Bonus momentum for headshots
					OwnerMomentumComp->AddMomentum(MomentumGainOnHit * 2.0f, FName("Headshot"));
				}
			}
		}

		// Play impact effect
		PlayEffect(ImpactEffect, ImpactSound, Hit.Location);

		// Explode if enabled
		if (bExplodeOnImpact)
		{
			Explode();
		}
		else
		{
			// Destroy the projectile
			Destroy();
		}
	}
}

void ARangedWeaponProjectile::ApplyDamage(AActor* Target, const FHitResult& Hit)
{
	if (!Target)
	{
		return;
	}

	// Apply damage to the target
	AActor* ProjectileOwner = GetOwner();
	AController* OwnerController = ProjectileOwner ? ProjectileOwner->GetInstigatorController() : nullptr;

	UGameplayStatics::ApplyPointDamage(
		Target,
		Damage,
		GetVelocity().GetSafeNormal(),
		Hit,
		OwnerController,
		this,
		UDamageType::StaticClass()
	);
}

void ARangedWeaponProjectile::CreateRiftAnchor(const FVector& Location)
{
	// Check if we have a valid anchor class
	if (!AnchorClass || !GetWorld())
	{
		return;
	}

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the anchor
	ARiftAnchor* NewAnchor = GetWorld()->SpawnActor<ARiftAnchor>(
		AnchorClass,
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	// Additional setup for the anchor can be done here
	if (NewAnchor)
	{
		// Set a limited duration for the anchor if desired
		// NewAnchor->SetLifeSpan(10.0f);
	}
}

void ARangedWeaponProjectile::TransformProjectile(EProjectileTransform TransformType)
{
	// Base implementation handles basic transformations
	switch (TransformType)
	{
	case EProjectileTransform::Split:
		// Splitting would be implemented in a derived class
		break;

	case EProjectileTransform::Homing:
		// Enable homing behavior
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingAccelerationMagnitude = 8000.0f;
		// Target acquisition would be handled in a derived class
		break;

	case EProjectileTransform::Explosive:
		// Make the projectile explosive
		bExplodeOnImpact = true;
		break;

	case EProjectileTransform::Piercing:
		// Enable piercing behavior
		// This would require custom collision handling in a derived class
		break;

	default:
		break;
	}
}

void ARangedWeaponProjectile::Explode()
{
	// Play explosion effect
	PlayEffect(ExplosionEffect, ExplosionSound, GetActorLocation());

	// Broadcast explode event
	OnProjectileExplode.Broadcast();

	// Apply radial damage
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	IgnoredActors.Add(GetOwner());
	
	// Get actors damaged by explosion
	TArray<AActor*> DamagedActors;
	UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		Damage,
		Damage * 0.5f, // Min damage at edge of radius
		GetActorLocation(),
		ExplosionRadius * 0.5f, // Inner radius
		ExplosionRadius, // Outer radius
		1.0f, // Damage falloff exponent
		UDamageType::StaticClass(),
		IgnoredActors,
		this,
		GetInstigatorController()
	);
	
	// Add momentum based on number of actors hit
	if (GetOwner())
	{
		UMomentumComponent* OwnerMomentumComp = GetOwner()->FindComponentByClass<UMomentumComponent>();
		if (OwnerMomentumComp)
		{
			// Get all actors in explosion radius
			TArray<AActor*> ActorsInRadius;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), ActorsInRadius);
			
			// Count how many valid targets were hit
			int32 HitCount = 0;
			for (AActor* Actor : ActorsInRadius)
			{
				if (Actor && Actor != this && Actor != GetOwner())
				{
					float Distance = FVector::Distance(GetActorLocation(), Actor->GetActorLocation());
					if (Distance <= ExplosionRadius)
					{
						HitCount++;
					}
				}
			}
			
			// Add momentum based on hit count (with diminishing returns)
			float ExplosionMomentum = MomentumGainOnHit * FMath::Min(HitCount, 5);
			OwnerMomentumComp->AddMomentum(ExplosionMomentum, FName("Explosion"));
			
			// Add bonus momentum for multi-kills
			if (HitCount >= 3)
			{
				OwnerMomentumComp->AddMomentum(MomentumGainOnHit * 3.0f, FName("MultiKill"));
			}
		}
	}

	// Destroy the projectile
	Destroy();
}

void ARangedWeaponProjectile::ApplyElementalEffect(AActor* Target)
{
	// Base implementation - derived classes can implement specific elemental effects
	switch (ElementalType)
	{
	case EElementalType::Fire:
		// Apply fire effect (could set the actor on fire)
		break;

	case EElementalType::Ice:
		// Apply ice effect (could slow the actor)
		break;

	case EElementalType::Electric:
		// Apply electric effect (could chain to nearby enemies)
		break;

	case EElementalType::Poison:
		// Apply poison effect (could apply damage over time)
		break;

	default:
		break;
	}
}

void ARangedWeaponProjectile::PlayEffect(UNiagaraSystem* Effect, USoundBase* Sound, const FVector& Location)
{
	// Play sound effect
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
	}

	// Spawn visual effect
	if (Effect && GetWorld())
	{
		// Use Niagara system if available
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Effect,
			Location,
			GetActorRotation()
		);
		
		// Alternatively, we could use a particle system here if Niagara is not available
		// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleEffect, Location, GetActorRotation());
	}
}