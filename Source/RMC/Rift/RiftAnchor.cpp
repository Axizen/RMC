// Fill out your copyright notice in the Description page of Project Settings.

#include "RiftAnchor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARiftAnchor::ARiftAnchor()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create and set up the visual component
	VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	VisualComponent->SetupAttachment(RootComponent);
	VisualComponent->SetCollisionProfileName(TEXT("NoCollision"));
	VisualComponent->SetGenerateOverlapEvents(false);
	VisualComponent->SetVisibility(true);

	// Create and set up the detection sphere
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetSphereRadius(200.0f);
	DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
	DetectionSphere->SetGenerateOverlapEvents(true);

	// Create and set up the VFX component
	RiftVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RiftVFX"));
	RiftVFX->SetupAttachment(RootComponent);
	RiftVFX->SetAutoActivate(true);

	// Default properties
	RequiredMomentum = 0.0f;
	bIsActive = true;
	SuggestedNextAnchor = nullptr;
}

// Called when the game starts or when spawned
void ARiftAnchor::BeginPlay()
{
	Super::BeginPlay();
	
	// Bind overlap events
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ARiftAnchor::OnDetectionOverlapBegin);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ARiftAnchor::OnDetectionOverlapEnd);

	// Update visuals based on initial state
	UpdateVisuals();
}

// Called every frame
void ARiftAnchor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARiftAnchor::OnRiftTo_Implementation()
{
	// Play a visual effect when rifted to
	if (RiftVFX)
	{
		RiftVFX->SetFloatParameter(TEXT("Intensity"), 2.0f);
		
		// Reset intensity after a short delay
		FTimerHandle ResetIntensityTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			ResetIntensityTimerHandle,
			[this]()
			{
				if (RiftVFX)
				{
					RiftVFX->SetFloatParameter(TEXT("Intensity"), 1.0f);
				}
			},
			0.5f,
			false);
	}
}

bool ARiftAnchor::CanUseWithMomentum(float CurrentMomentum) const
{
	// Check if the anchor is active and the momentum requirement is met
	return bIsActive && CurrentMomentum >= RequiredMomentum;
}

void ARiftAnchor::Activate()
{
	bIsActive = true;
	UpdateVisuals();
}

void ARiftAnchor::Deactivate()
{
	bIsActive = false;
	UpdateVisuals();
}

void ARiftAnchor::OnDetectionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// This will be used to highlight the anchor when a player is nearby
	if (bIsActive && RiftVFX)
	{
		RiftVFX->SetFloatParameter(TEXT("Highlight"), 1.0f);
	}
}

void ARiftAnchor::OnDetectionOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Remove highlight when player leaves
	if (RiftVFX)
	{
		RiftVFX->SetFloatParameter(TEXT("Highlight"), 0.0f);
	}
}

void ARiftAnchor::UpdateVisuals()
{
	// Update visual appearance based on active state
	if (VisualComponent)
	{
		VisualComponent->SetVisibility(bIsActive);
	}

	if (RiftVFX)
	{
		RiftVFX->SetFloatParameter(TEXT("Active"), bIsActive ? 1.0f : 0.0f);
	}
}