// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponEnums.h"
#include "NiagaraSystem.h"
#include "RangedWeaponProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UNiagaraComponent;
class ARiftAnchor;

UCLASS()
class RMC_API ARangedWeaponProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARangedWeaponProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* EffectComponent;
	
	// Properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCanRiftTo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCreateAnchorOnImpact;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCanJuggleTarget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float JuggleForce;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float StylePointsOnHit;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MomentumGainOnHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float LifeSpan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bExplodeOnImpact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ExplosionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EElementalType ElementalType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ElementalEffectChance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<ARiftAnchor> AnchorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	UNiagaraSystem* ImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	USoundBase* ImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	UNiagaraSystem* ExplosionEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	USoundBase* ExplosionSound;
	
	// Functions
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void ApplyDamage(AActor* Target, const FHitResult& Hit);
	
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void CreateRiftAnchor(const FVector& Location);
	
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void TransformProjectile(EProjectileTransform TransformType);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void Explode();

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	virtual void ApplyElementalEffect(AActor* Target);

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileHit, const FHitResult&, Hit);
	UPROPERTY(BlueprintAssignable, Category = "Projectile|Events")
	FOnProjectileHit OnProjectileHit;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProjectileExplode);
	UPROPERTY(BlueprintAssignable, Category = "Projectile|Events")
	FOnProjectileExplode OnProjectileExplode;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Helper function to play effects
	void PlayEffect(UNiagaraSystem* Effect, USoundBase* Sound, const FVector& Location);
};