// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "RailSplineActor.generated.h"

UENUM(BlueprintType)
enum class ERailType : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Boost UMETA(DisplayName = "Boost"),
	Trick UMETA(DisplayName = "Trick"),
	Danger UMETA(DisplayName = "Danger")
};

UCLASS()
class RMC_API ARailSplineActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARailSplineActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Spline component that defines the rail path
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rail")
	USplineComponent* RailSpline;

	// Mesh component for the rail
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rail")
	UStaticMeshComponent* RailMesh;

	// Type of rail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	ERailType RailType;

	// Speed multiplier for this rail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float SpeedMultiplier;

	// Momentum gain multiplier for this rail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float MomentumGainMultiplier;

	// Style points multiplier for this rail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	float StylePointsMultiplier;

	// Whether this rail has a boost section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	bool bHasBoostSection;

	// Range of the boost section (0.0 to 1.0 along spline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail", meta = (EditCondition = "bHasBoostSection"))
	FVector2D BoostSectionRange;

	// Boost multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail", meta = (EditCondition = "bHasBoostSection"))
	float BoostMultiplier;

	// Whether this rail has a trick section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	bool bHasTrickSection;

	// Range of the trick section (0.0 to 1.0 along spline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail", meta = (EditCondition = "bHasTrickSection"))
	FVector2D TrickSectionRange;

	// Whether this rail has a danger section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail")
	bool bHasDangerSection;

	// Range of the danger section (0.0 to 1.0 along spline)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rail", meta = (EditCondition = "bHasDangerSection"))
	FVector2D DangerSectionRange;

	// Gets the closest point on the rail to a given location
	UFUNCTION(BlueprintCallable, Category = "Rail")
	FVector GetClosestPointOnRail(const FVector& Location, float& OutDistanceAlongRail, float& OutDistanceFromRail) const;

	// Gets the rail type at a specific distance along the rail
	UFUNCTION(BlueprintCallable, Category = "Rail")
	ERailType GetRailTypeAtDistance(float Distance) const;

	// Gets the speed multiplier at a specific distance along the rail
	UFUNCTION(BlueprintCallable, Category = "Rail")
	float GetSpeedMultiplierAtDistance(float Distance) const;

	// Gets the total length of the rail
	UFUNCTION(BlueprintCallable, Category = "Rail")
	float GetRailLength() const;

	// Gets the direction of the rail at a specific distance
	UFUNCTION(BlueprintCallable, Category = "Rail")
	FVector GetDirectionAtDistance(float Distance) const;

	// Gets the position on the rail at a specific distance
	UFUNCTION(BlueprintCallable, Category = "Rail")
	FVector GetPositionAtDistance(float Distance) const;

	// Gets the up vector of the rail at a specific distance
	UFUNCTION(BlueprintCallable, Category = "Rail")
	FVector GetUpVectorAtDistance(float Distance) const;

	// Gets the right vector of the rail at a specific distance
	UFUNCTION(BlueprintCallable, Category = "Rail")
	FVector GetRightVectorAtDistance(float Distance) const;

	// Gets the closest rail to this one within a certain distance
	UFUNCTION(BlueprintCallable, Category = "Rail")
	ARailSplineActor* GetClosestRail(float MaxDistance, const FVector& ReferenceLocation) const;

	// Gets all rails within a certain distance
	UFUNCTION(BlueprintCallable, Category = "Rail")
	TArray<ARailSplineActor*> GetRailsInRange(float MaxDistance, const FVector& ReferenceLocation) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Helper function to check if a distance is within a range
	bool IsDistanceInRange(float Distance, const FVector2D& Range) const;
};