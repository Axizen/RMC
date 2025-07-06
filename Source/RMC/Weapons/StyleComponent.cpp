// Fill out your copyright notice in the Description page of Project Settings.

#include "StyleComponent.h"
#include "../Core/ComponentLocator.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UStyleComponent::UStyleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	CurrentStylePoints = 0.0f;
	CurrentStyleRank = EStyleRank::D;
	StyleDecayRate = 5.0f;
	StylePointCap = 5000.0f;
	NoHitDecayMultiplier = 2.0f;
	TimeSinceLastStyleGain = 0.0f;
	NoHitDecayDelay = 3.0f;

	// Default rank thresholds
	StyleRankThresholds.Add(EStyleRank::D, 0.0f);
	StyleRankThresholds.Add(EStyleRank::C, 100.0f);
	StyleRankThresholds.Add(EStyleRank::B, 500.0f);
	StyleRankThresholds.Add(EStyleRank::A, 1000.0f);
	StyleRankThresholds.Add(EStyleRank::S, 2000.0f);
	StyleRankThresholds.Add(EStyleRank::SS, 3000.0f);
	StyleRankThresholds.Add(EStyleRank::SSS, 4000.0f);

	// Default move values
	MoveStyleValues.Add(FName("BasicShot"), 10.0f);
	MoveStyleValues.Add(FName("ChargedShot"), 30.0f);
	MoveStyleValues.Add(FName("PhantomDodge"), 20.0f);
	MoveStyleValues.Add(FName("RiftTether"), 25.0f);
	MoveStyleValues.Add(FName("ChainRift"), 40.0f);
	MoveStyleValues.Add(FName("CounterRift"), 50.0f);
	MoveStyleValues.Add(FName("Headshot"), 50.0f);
	MoveStyleValues.Add(FName("MultiKill"), 100.0f);
	MoveStyleValues.Add(FName("WallRun"), 15.0f);
	MoveStyleValues.Add(FName("WallJump"), 25.0f);

	// Default diminishing returns
	MoveDiminishingReturns.Add(FName("BasicShot"), 0.8f);
	MoveDiminishingReturns.Add(FName("ChargedShot"), 0.9f);
	MoveDiminishingReturns.Add(FName("PhantomDodge"), 0.7f);
	MoveDiminishingReturns.Add(FName("RiftTether"), 0.8f);
	MoveDiminishingReturns.Add(FName("ChainRift"), 0.9f);
	MoveDiminishingReturns.Add(FName("CounterRift"), 0.95f);
	MoveDiminishingReturns.Add(FName("Headshot"), 0.9f);
	MoveDiminishingReturns.Add(FName("MultiKill"), 1.0f);
	MoveDiminishingReturns.Add(FName("WallRun"), 0.85f);
	MoveDiminishingReturns.Add(FName("WallJump"), 0.9f);
}

// Called when the game starts
void UStyleComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Get the event system
	EventSystem = GetEventSystem();
	
	// Register event listeners
	RegisterEventListeners();
}

// Interface implementations
float UStyleComponent::GetCurrentStylePoints_Implementation() const
{
	return CurrentStylePoints;
}

EStyleRank UStyleComponent::GetCurrentStyleRank_Implementation() const
{
	return CurrentStyleRank;
}

float UStyleComponent::GetStyleMultiplier_Implementation() const
{
	return GetStyleMultiplierInternal();
}

float UStyleComponent::GetStyleRankProgress_Implementation() const
{
	return GetStyleRankProgressInternal();
}

float UStyleComponent::GetMoveStyleValue_Implementation(FName MoveName) const
{
	return GetMoveStyleValueInternal(MoveName);
}

void UStyleComponent::AddStylePoints_Implementation(float Points, FName MoveName)
{
	AddStylePointsInternal(Points, MoveName);
}

// Called every frame
void UStyleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Track time since last style gain
	TimeSinceLastStyleGain += DeltaTime;

	// Decay style over time
	DecayStyle(DeltaTime);
}

void UStyleComponent::OnRegister()
{
	Super::OnRegister();
	
	// Register with the component locator
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->RegisterComponent(this, FName("StyleComponent"));
		}
	}
}

void UStyleComponent::OnUnregister()
{
	// Unregister from the component locator
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->UnregisterComponent(this);
		}
	}
	
	Super::OnUnregister();
}

void UStyleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister event listeners
	UnregisterEventListeners();
	
	Super::EndPlay(EndPlayReason);
}

UGameEventSubsystem* UStyleComponent::GetEventSystem() const
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	if (GameInstance)
	{
		return GameInstance->GetSubsystem<UGameEventSubsystem>();
	}
	return nullptr;
}

void UStyleComponent::RegisterEventListeners()
{
	if (!EventSystem)
	{
		return;
	}
	
	// Register for player damaged events
	int32 Handle = EventSystem->AddEventListenerWithObject(EGameEventType::PlayerDamaged, const_cast<UStyleComponent*>(this), FName("OnPlayerDamaged"));
	EventListenerHandles.Add(Handle);
	
	// Register for enemy defeated events
	Handle = EventSystem->AddEventListenerWithObject(EGameEventType::EnemyDefeated, const_cast<UStyleComponent*>(this), FName("OnEnemyDefeated"));
	EventListenerHandles.Add(Handle);
	
	// Register for weapon fired events
	Handle = EventSystem->AddEventListenerWithObject(EGameEventType::WeaponFired, const_cast<UStyleComponent*>(this), FName("OnWeaponFired"));
	EventListenerHandles.Add(Handle);
}

void UStyleComponent::UnregisterEventListeners()
{
	if (!EventSystem)
	{
		return;
	}
	
	// Remove all event listeners
	for (int32 Handle : EventListenerHandles)
	{
		EventSystem->RemoveEventListener(Handle);
	}
	EventListenerHandles.Empty();
}

void UStyleComponent::AddStylePointsInternal(float Points, FName MoveName)
{
	// Apply diminishing returns for repeated moves
	float AdjustedPoints = CalculateDiminishingReturns(MoveName, Points);

	// Add points
	float OldPoints = CurrentStylePoints;
	CurrentStylePoints = FMath::Min(CurrentStylePoints + AdjustedPoints, StylePointCap);

	// Reset time since last style gain
	TimeSinceLastStyleGain = 0.0f;

	// Update last use time for this move
	LastMoveUseTimes.Add(MoveName, GetWorld()->GetTimeSeconds());

	// Update style rank
	UpdateStyleRank();

	// Broadcast points changed event
	OnStylePointsChanged.Broadcast(CurrentStylePoints, CurrentStylePoints - OldPoints);
	
	// Broadcast through the event system
	BroadcastStylePointsGainedEvent(AdjustedPoints, MoveName);
}

void UStyleComponent::DecayStyle(float DeltaTime)
{
	// Calculate decay rate based on time since last style gain
	float CurrentDecayRate = StyleDecayRate;
	if (TimeSinceLastStyleGain > NoHitDecayDelay)
	{
		// Increase decay rate if player hasn't gained style in a while
		CurrentDecayRate *= NoHitDecayMultiplier;
	}

	// Apply decay
	float OldPoints = CurrentStylePoints;
	CurrentStylePoints = FMath::Max(CurrentStylePoints - (CurrentDecayRate * DeltaTime), 0.0f);

	// Update style rank if points changed
	if (OldPoints != CurrentStylePoints)
	{
		UpdateStyleRank();

		// Broadcast points changed event
		OnStylePointsChanged.Broadcast(CurrentStylePoints, CurrentStylePoints - OldPoints);
	}
}

void UStyleComponent::ResetStyle()
{
	// Store old values for event broadcasting
	float OldPoints = CurrentStylePoints;
	EStyleRank OldRank = CurrentStyleRank;
	
	// Reset to default values
	CurrentStylePoints = 0.0f;
	CurrentStyleRank = EStyleRank::D;
	TimeSinceLastStyleGain = 0.0f;
	LastMoveUseTimes.Empty();

	// Broadcast events through delegates
	OnStylePointsChanged.Broadcast(CurrentStylePoints, -OldPoints);
	if (OldRank != CurrentStyleRank)
	{
		OnStyleRankChanged.Broadcast(CurrentStyleRank);
	}
	
	// Broadcast through the event system
	BroadcastStylePointsGainedEvent(-OldPoints, FName("ResetStyle"));
	if (OldRank != CurrentStyleRank)
	{
		BroadcastStyleRankChangedEvent(CurrentStyleRank);
	}
}

float UStyleComponent::GetStyleMultiplierInternal() const
{
	// Return a multiplier based on current rank
	switch (CurrentStyleRank)
	{
	case EStyleRank::D: return 1.0f;
	case EStyleRank::C: return 1.1f;
	case EStyleRank::B: return 1.25f;
	case EStyleRank::A: return 1.5f;
	case EStyleRank::S: return 1.75f;
	case EStyleRank::SS: return 2.0f;
	case EStyleRank::SSS: return 2.5f;
	default: return 1.0f;
	}
}

float UStyleComponent::GetStyleRankProgressInternal() const
{
	// Get thresholds for current and next rank
	float CurrentThreshold = GetThresholdForRank(CurrentStyleRank);
	EStyleRank NextRank = (EStyleRank)(((uint8)CurrentStyleRank) + 1);
	float NextThreshold = GetThresholdForRank(NextRank);

	// If we're at max rank, return 1.0
	if (NextRank > EStyleRank::SSS)
	{
		return 1.0f;
	}

	// Calculate progress as percentage between current and next rank
	float RankRange = NextThreshold - CurrentThreshold;
	return (CurrentStylePoints - CurrentThreshold) / RankRange;
}

FText UStyleComponent::GetStyleRankText() const
{
	// Convert rank enum to text
	switch (CurrentStyleRank)
	{
	case EStyleRank::D: return FText::FromString("D");
	case EStyleRank::C: return FText::FromString("C");
	case EStyleRank::B: return FText::FromString("B");
	case EStyleRank::A: return FText::FromString("A");
	case EStyleRank::S: return FText::FromString("S");
	case EStyleRank::SS: return FText::FromString("SS");
	case EStyleRank::SSS: return FText::FromString("SSS");
	default: return FText::FromString("D");
	}
}

float UStyleComponent::GetMoveStyleValueInternal(FName MoveName) const
{
	// Return the base style value for a move
	if (MoveStyleValues.Contains(MoveName))
	{
		return MoveStyleValues[MoveName];
	}
	return 0.0f;
}

void UStyleComponent::TakeDamage(float DamageAmount)
{
	// Taking damage reduces style points
	float StylePenalty = DamageAmount * 2.0f; // Adjust multiplier as needed
	float OldPoints = CurrentStylePoints;
	CurrentStylePoints = FMath::Max(CurrentStylePoints - StylePenalty, 0.0f);

	// Update style rank
	UpdateStyleRank();

	// Broadcast points changed event
	OnStylePointsChanged.Broadcast(CurrentStylePoints, CurrentStylePoints - OldPoints);
	
	// Broadcast through the event system
	BroadcastStylePointsGainedEvent(-StylePenalty, FName("TakeDamage"));
}

void UStyleComponent::UpdateStyleRank()
{
	// Determine new rank based on current points
	EStyleRank NewRank = EStyleRank::D;

	if (CurrentStylePoints >= GetThresholdForRank(EStyleRank::SSS))
	{
		NewRank = EStyleRank::SSS;
	}
	else if (CurrentStylePoints >= GetThresholdForRank(EStyleRank::SS))
	{
		NewRank = EStyleRank::SS;
	}
	else if (CurrentStylePoints >= GetThresholdForRank(EStyleRank::S))
	{
		NewRank = EStyleRank::S;
	}
	else if (CurrentStylePoints >= GetThresholdForRank(EStyleRank::A))
	{
		NewRank = EStyleRank::A;
	}
	else if (CurrentStylePoints >= GetThresholdForRank(EStyleRank::B))
	{
		NewRank = EStyleRank::B;
	}
	else if (CurrentStylePoints >= GetThresholdForRank(EStyleRank::C))
	{
		NewRank = EStyleRank::C;
	}

	// If rank changed, broadcast event
	if (NewRank != CurrentStyleRank)
	{
		CurrentStyleRank = NewRank;
		OnStyleRankChanged.Broadcast(CurrentStyleRank);
		
		// Broadcast through the event system
		BroadcastStyleRankChangedEvent(CurrentStyleRank);
	}
}

// Event handlers
void UStyleComponent::OnPlayerDamaged(const FGameEventData& EventData)
{
	// Taking damage reduces style points
	if (EventData.Instigator == GetOwner())
	{
		// Only process if this is our owner
		TakeDamage(EventData.FloatValue);
	}
}

void UStyleComponent::OnEnemyDefeated(const FGameEventData& EventData)
{
	// Defeating enemies adds style points
	if (EventData.Instigator == GetOwner())
	{
		// Add style points for defeating an enemy
		AddStylePointsInternal(100.0f, FName("EnemyDefeated"));
	}
}

void UStyleComponent::OnWeaponFired(const FGameEventData& EventData)
{
	// Add style points for weapon firing
	if (EventData.Instigator == GetOwner())
	{
		// Add a small amount of style points for firing a weapon
		AddStylePointsInternal(5.0f, EventData.NameValue);
	}
}

// Event broadcasting
void UStyleComponent::BroadcastStylePointsGainedEvent(float Points, FName MoveName)
{
	if (!EventSystem)
	{
		return;
	}
	
	// Create and broadcast style points gained event
	FGameEventData EventData;
	EventData.EventType = EGameEventType::StylePointsGained;
	EventData.Instigator = GetOwner();
	EventData.FloatValue = Points;
	EventData.NameValue = MoveName;
	
	EventSystem->BroadcastEvent(EventData);
}

void UStyleComponent::BroadcastStyleRankChangedEvent(EStyleRank NewRank)
{
	if (!EventSystem)
	{
		return;
	}
	
	// Create and broadcast style rank changed event
	FGameEventData EventData;
	EventData.EventType = EGameEventType::StyleRankChanged;
	EventData.Instigator = GetOwner();
	EventData.IntValue = static_cast<int32>(NewRank);
	
	EventSystem->BroadcastEvent(EventData);
}

float UStyleComponent::CalculateDiminishingReturns(FName MoveName, float BasePoints)
{
	// If move doesn't have diminishing returns, return base points
	if (!MoveDiminishingReturns.Contains(MoveName))
	{
		return BasePoints;
	}

	// Get diminishing return factor
	float DiminishingFactor = MoveDiminishingReturns[MoveName];

	// Get time since last use of this move
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float LastUseTime = LastMoveUseTimes.Contains(MoveName) ? LastMoveUseTimes[MoveName] : 0.0f;
	float TimeSinceLastUse = CurrentTime - LastUseTime;

	// If it's been a while since last use, no diminishing returns
	if (TimeSinceLastUse > 5.0f) // 5 seconds threshold
	{
		return BasePoints;
	}

	// Calculate diminishing returns based on time since last use
	// More recent = more diminishing
	float TimeBasedFactor = FMath::Min(TimeSinceLastUse / 5.0f, 1.0f);
	float EffectiveFactor = FMath::Lerp(DiminishingFactor, 1.0f, TimeBasedFactor);

	return BasePoints * EffectiveFactor;
}

float UStyleComponent::GetThresholdForRank(EStyleRank Rank) const
{
	// Return threshold for the specified rank
	if (StyleRankThresholds.Contains(Rank))
	{
		return StyleRankThresholds[Rank];
	}
	return 0.0f;
}