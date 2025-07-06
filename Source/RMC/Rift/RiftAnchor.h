// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RiftAnchor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UNiagaraComponent;

UCLASS()
class RMC_API ARiftAnchor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARiftAnchor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Visual component to show the anchor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	// Collision sphere to detect player proximity
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* DetectionSphere;

	// VFX component for the rift anchor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* RiftVFX;

	// Whether this anchor requires a minimum momentum to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	float RequiredMomentum;

	// Whether this anchor is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rift")
	bool bIsActive;

	// The next suggested anchor in a chain
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Rift")
	ARiftAnchor* SuggestedNextAnchor;

	// Called when a character successfully rifts to this anchor
	UFUNCTION(BlueprintNativeEvent, Category = "Rift")
	void OnRiftTo();
	virtual void OnRiftTo_Implementation();

	// Returns whether this anchor can be used with the given momentum
	UFUNCTION(BlueprintPure, Category = "Rift")
	bool CanUseWithMomentum(float CurrentMomentum) const;

	// Returns the next anchor in the chain if available
	UFUNCTION(BlueprintPure, Category = "Rift")
	ARiftAnchor* GetNextAnchor() const { return SuggestedNextAnchor; }

	// Activates this anchor
	UFUNCTION(BlueprintCallable, Category = "Rift")
	void Activate();

	// Deactivates this anchor
	UFUNCTION(BlueprintCallable, Category = "Rift")
	void Deactivate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when an actor enters the detection sphere
	UFUNCTION()
	void OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when an actor exits the detection sphere
	UFUNCTION()
	void OnDetectionOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	// Updates the visual appearance based on active state
	void UpdateVisuals();
};