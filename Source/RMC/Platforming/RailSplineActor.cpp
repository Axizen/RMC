// Fill out your copyright notice in the Description page of Project Settings.

#include "RailSplineActor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARailSplineActor::ARailSplineActor()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Create the spline component
	RailSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RailSpline"));
	RootComponent = RailSpline;

	// Create the rail mesh component
	RailMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RailMesh"));
	RailMesh->SetupAttachment(RailSpline);
	
	// Set default values
	RailType = ERailType::Normal;
	SpeedMultiplier = 1.0f;
	MomentumGainMultiplier = 1.0f;
	StylePointsMultiplier = 1.0f;
	bHasBoostSection = false;
	BoostSectionRange = FVector2D(0.3f, 0.6f);
	BoostMultiplier = 1.5f;
	bHasTrickSection = false;
	TrickSectionRange = FVector2D(0.4f, 0.7f);
	bHasDangerSection = false;
	DangerSectionRange = FVector2D(0.8f, 0.9f);
}

// Called when the game starts or when spawned
void ARailSplineActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARailSplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector ARailSplineActor::GetClosestPointOnRail(const FVector& Location, float& OutDistanceAlongRail, float& OutDistanceFromRail) const
{
	// Get the closest point on the spline
	const float Key = RailSpline->FindInputKeyClosestToWorldLocation(Location);
	const FVector ClosestPoint = RailSpline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);
	
	// Calculate the distance along the rail
	OutDistanceAlongRail = RailSpline->GetDistanceAlongSplineAtSplineInputKey(Key);
	
	// Calculate the distance from the rail
	OutDistanceFromRail = FVector::Distance(Location, ClosestPoint);
	
	return ClosestPoint;
}

ERailType ARailSplineActor::GetRailTypeAtDistance(float Distance) const
{
	// Normalize the distance to a 0-1 range
	const float NormalizedDistance = Distance / GetRailLength();
	
	// Check if we're in a special section
	if (bHasBoostSection && IsDistanceInRange(NormalizedDistance, BoostSectionRange))
	{
		return ERailType::Boost;
	}
	else if (bHasTrickSection && IsDistanceInRange(NormalizedDistance, TrickSectionRange))
	{
		return ERailType::Trick;
	}
	else if (bHasDangerSection && IsDistanceInRange(NormalizedDistance, DangerSectionRange))
	{
		return ERailType::Danger;
	}
	
	return RailType;
}

float ARailSplineActor::GetSpeedMultiplierAtDistance(float Distance) const
{
	// Normalize the distance to a 0-1 range
	const float NormalizedDistance = Distance / GetRailLength();
	
	// Check if we're in a boost section
	if (bHasBoostSection && IsDistanceInRange(NormalizedDistance, BoostSectionRange))
	{
		return SpeedMultiplier * BoostMultiplier;
	}
	
	return SpeedMultiplier;
}

float ARailSplineActor::GetRailLength() const
{
	return RailSpline->GetSplineLength();
}

FVector ARailSplineActor::GetDirectionAtDistance(float Distance) const
{
	const float Key = RailSpline->SplineReparamTable.InaccurateFindNearest(Distance);
	return RailSpline->GetDirectionAtSplineInputKey(Key, ESplineCoordinateSpace::World);
}

FVector ARailSplineActor::GetPositionAtDistance(float Distance) const
{
	const float Key = RailSpline->SplineReparamTable.InaccurateFindNearest(Distance);
	return RailSpline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);
}

FVector ARailSplineActor::GetUpVectorAtDistance(float Distance) const
{
	const float Key = RailSpline->SplineReparamTable.InaccurateFindNearest(Distance);
	return RailSpline->GetUpVectorAtSplineInputKey(Key, ESplineCoordinateSpace::World);
}

FVector ARailSplineActor::GetRightVectorAtDistance(float Distance) const
{
	const float Key = RailSpline->SplineReparamTable.InaccurateFindNearest(Distance);
	return RailSpline->GetRightVectorAtSplineInputKey(Key, ESplineCoordinateSpace::World);
}

ARailSplineActor* ARailSplineActor::GetClosestRail(float MaxDistance, const FVector& ReferenceLocation) const
{
	// Get all rails in the level
	TArray<AActor*> Rails;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARailSplineActor::StaticClass(), Rails);
	
	// Find the closest rail that isn't this one
	ARailSplineActor* ClosestRail = nullptr;
	float ClosestDistance = MaxDistance;
	
	for (AActor* Rail : Rails)
	{
		// Skip this rail
		if (Rail == this)
		{
			continue;
		}
		
		// Cast to rail spline actor
		ARailSplineActor* RailActor = Cast<ARailSplineActor>(Rail);
		if (!RailActor)
		{
			continue;
		}
		
		// Get the closest point on the rail
		float DistanceAlongRail, DistanceFromRail;
		FVector ClosestPoint = RailActor->GetClosestPointOnRail(ReferenceLocation, DistanceAlongRail, DistanceFromRail);
		
		// Check if this rail is closer
		if (DistanceFromRail < ClosestDistance)
		{
			ClosestRail = RailActor;
			ClosestDistance = DistanceFromRail;
		}
	}
	
	return ClosestRail;
}

TArray<ARailSplineActor*> ARailSplineActor::GetRailsInRange(float MaxDistance, const FVector& ReferenceLocation) const
{
	// Get all rails in the level
	TArray<AActor*> Rails;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARailSplineActor::StaticClass(), Rails);
	
	// Find all rails within range
	TArray<ARailSplineActor*> RailsInRange;
	
	for (AActor* Rail : Rails)
	{
		// Skip this rail
		if (Rail == this)
		{
			continue;
		}
		
		// Cast to rail spline actor
		ARailSplineActor* RailActor = Cast<ARailSplineActor>(Rail);
		if (!RailActor)
		{
			continue;
		}
		
		// Get the closest point on the rail
		float DistanceAlongRail, DistanceFromRail;
		FVector ClosestPoint = RailActor->GetClosestPointOnRail(ReferenceLocation, DistanceAlongRail, DistanceFromRail);
		
		// Check if this rail is within range
		if (DistanceFromRail <= MaxDistance)
		{
			RailsInRange.Add(RailActor);
		}
	}
	
	return RailsInRange;
}

bool ARailSplineActor::IsDistanceInRange(float Distance, const FVector2D& Range) const
{
	return Distance >= Range.X && Distance <= Range.Y;
}