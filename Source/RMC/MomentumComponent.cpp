// Fill out your copyright notice in the Description page of Project Settings.

#include "MomentumComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/GameEventSystem.h"
#include "Core/ComponentLocator.h"
#include "Subsystems/GameInstanceSubsystem.h"

// Sets default values for this component's properties
UMomentumComponent::UMomentumComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	CurrentMomentum = 0.0f;
	MaxMomentum = 100.0f;
	MomentumDecayRate = 5.0f;
	MomentumGainMultiplier = 1.0f;
	BankedMomentum = 0.0f;
	MaxBankedMomentum = 50.0f;
	bIsOvercharged = false;
	bWasOvercharged = false;
	OverchargeThreshold = 0.95f;
	OverchargeDamageMultiplier = 1.5f;
	OverchargeDamageTakenMultiplier = 1.3f;
	PreviousMomentumTier = 0;
	LastMomentumGainTime = 0.0f;

	// Default momentum thresholds
	MomentumThresholds = { 20.0f, 40.0f, 60.0f, 80.0f };

	// Platforming-specific default values
	WallRunMomentumGain = 10.0f;
	DoubleJumpMomentumGain = 5.0f;
	WallBounceMomentumGain = 15.0f;
	RailGrindMomentumGain = 8.0f;
	RiftChainMomentumGain = 12.0f;
	bMomentumPreservationActive = false;
	MaxMomentumPreservationTime = 2.0f;
	MomentumPreservationStartTime = 0.0f;
	CurrentMomentumPreservationDuration = 0.0f;
	LastMomentumPulseTime = 0.0f;
	MomentumPulseCooldown = 0.5f;

	// Default platforming abilities
	PlatformingAbilities.Add(FPlatformingAbilityThreshold(FName("DoubleJump"), 20.0f, 0.0f));
	PlatformingAbilities.Add(FPlatformingAbilityThreshold(FName("WallBounce"), 30.0f, 1.0f));
	PlatformingAbilities.Add(FPlatformingAbilityThreshold(FName("AerialDash"), 40.0f, 2.0f));
	PlatformingAbilities.Add(FPlatformingAbilityThreshold(FName("ExtendedWallRun"), 50.0f, 0.0f, true, 2));
	PlatformingAbilities.Add(FPlatformingAbilityThreshold(FName("InfiniteRiftChain"), 80.0f, 0.0f, true, 3));
}

// Called when the component is registered
void UMomentumComponent::OnRegister()
{
	Super::OnRegister();
	
	// Register with the component locator if available
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->RegisterComponent(this, FName("MomentumComponent"));
		}
	}
}

// Called when the component is unregistered
void UMomentumComponent::OnUnregister()
{
	// Unregister from the component locator if available
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

// Called when the game starts
void UMomentumComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize last momentum gain time
	LastMomentumGainTime = GetWorld()->GetTimeSeconds();
	
	// Get the event subsystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance)
	{
		EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
		if (EventSystem)
		{
			RegisterForEvents();
		}
	}
}

// Called when the component is destroyed
void UMomentumComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from events
	UnregisterFromEvents();
	
	Super::EndPlay(EndPlayReason);
}

// Register for game events
void UMomentumComponent::RegisterForEvents()
{
	if (!EventSystem)
	{
		return;
	}
	
	// Register for style points gained events
	int32 Handle = EventSystem->AddEventListenerWithObject(EGameEventType::StylePointsGained, this, FName("OnStylePointsGained"));
	EventListenerHandles.Add(Handle);
	
	// Register for player damaged events
	Handle = EventSystem->AddEventListenerWithObject(EGameEventType::PlayerDamaged, this, FName("OnPlayerDamaged"));
	EventListenerHandles.Add(Handle);
	
	// Register for enemy defeated events
	Handle = EventSystem->AddEventListenerWithObject(EGameEventType::EnemyDefeated, this, FName("OnEnemyDefeated"));
	EventListenerHandles.Add(Handle);
	
	// Register for weapon fired events
	Handle = EventSystem->AddEventListenerWithObject(EGameEventType::WeaponFired, this, FName("OnWeaponFired"));
	EventListenerHandles.Add(Handle);
}

// Unregister from game events
void UMomentumComponent::UnregisterFromEvents()
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

// Called every frame
void UMomentumComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only decay momentum if preservation is not active
	if (!bMomentumPreservationActive)
	{
		DecayMomentum(DeltaTime);
	}

	// Update overcharge state
	UpdateOverchargeState();

	// Update platforming ability availability
	UpdatePlatformingAbilityAvailability();

	// Update ability cooldowns
	float CurrentTime = GetWorld()->GetTimeSeconds();
	TArray<FName> ExpiredCooldowns;

	for (auto& Pair : PlatformingAbilityCooldowns)
	{
		if (CurrentTime >= Pair.Value)
		{
			ExpiredCooldowns.Add(Pair.Key);
		}
	}

	// Remove expired cooldowns
	for (const FName& AbilityName : ExpiredCooldowns)
	{
		PlatformingAbilityCooldowns.Remove(AbilityName);
	}
}

void UMomentumComponent::AddMomentum_Implementation(float Amount, FName SourceName)
{
	// Apply diminishing returns for repeated sources
	float AdjustedAmount = CalculateDiminishingReturns(SourceName, Amount);

	// Apply momentum gain multiplier
	AdjustedAmount *= MomentumGainMultiplier;

	// Store old momentum for delta calculation
	float OldMomentum = CurrentMomentum;

	// Add momentum, clamped to max
	CurrentMomentum = FMath::Clamp(CurrentMomentum + AdjustedAmount, 0.0f, MaxMomentum);

	// Update last gain time
	LastMomentumGainTime = GetWorld()->GetTimeSeconds();

	// Update source last use time
	MomentumSourceLastUseTimes.Add(SourceName, LastMomentumGainTime);

	// Check for tier change
	int32 NewTier = GetMomentumTier_Implementation();
	if (NewTier != PreviousMomentumTier)
	{
		PreviousMomentumTier = NewTier;
		
		// Broadcast through delegate (legacy support)
		OnMomentumTierChanged.Broadcast(NewTier);
		
		// Broadcast through event system
		BroadcastMomentumTierChanged(NewTier);
	}

	// Broadcast momentum changed event
	OnMomentumChanged.Broadcast(CurrentMomentum, CurrentMomentum - OldMomentum);
	
	// Broadcast through event system
	BroadcastMomentumChanged(CurrentMomentum, CurrentMomentum - OldMomentum);
}

void UMomentumComponent::DecayMomentum(float DeltaTime)
{
	// Calculate time since last gain
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float TimeSinceLastGain = CurrentTime - LastMomentumGainTime;

	// Decay rate increases the longer we go without gaining momentum
	float DecayMultiplier = 1.0f;
	if (TimeSinceLastGain > 3.0f) // After 3 seconds, start increasing decay
	{
		DecayMultiplier = 1.0f + ((TimeSinceLastGain - 3.0f) * 0.1f); // 10% increase per second after 3 seconds
		DecayMultiplier = FMath::Min(DecayMultiplier, 3.0f); // Cap at 3x decay rate
	}

	// Store old momentum for delta calculation
	float OldMomentum = CurrentMomentum;

	// Apply decay
	CurrentMomentum = FMath::Max(CurrentMomentum - (MomentumDecayRate * DecayMultiplier * DeltaTime), 0.0f);

	// Check for tier change
	int32 NewTier = GetMomentumTier_Implementation();
	if (NewTier != PreviousMomentumTier)
	{
		PreviousMomentumTier = NewTier;
		
		// Broadcast through delegate (legacy support)
		OnMomentumTierChanged.Broadcast(NewTier);
		
		// Broadcast through event system
		BroadcastMomentumTierChanged(NewTier);
	}

	// Only broadcast if momentum actually changed
	if (!FMath::IsNearlyEqual(CurrentMomentum, OldMomentum))
	{
		// Broadcast through delegate (legacy support)
		OnMomentumChanged.Broadcast(CurrentMomentum, CurrentMomentum - OldMomentum);
		
		// Broadcast through event system
		BroadcastMomentumChanged(CurrentMomentum, CurrentMomentum - OldMomentum);
	}
}

bool UMomentumComponent::SpendMomentum_Implementation(float Amount)
{
	// Check if we have enough momentum
	if (CurrentMomentum >= Amount)
	{
		// Store old momentum for delta calculation
		float OldMomentum = CurrentMomentum;

		// Spend momentum
		CurrentMomentum -= Amount;

		// Check for tier change
		int32 NewTier = GetMomentumTier_Implementation();
		if (NewTier != PreviousMomentumTier)
		{
			PreviousMomentumTier = NewTier;
			
			// Broadcast through delegate (legacy support)
			OnMomentumTierChanged.Broadcast(NewTier);
			
			// Broadcast through event system
			BroadcastMomentumTierChanged(NewTier);
		}

		// Broadcast momentum changed event
		OnMomentumChanged.Broadcast(CurrentMomentum, CurrentMomentum - OldMomentum);
		
		// Broadcast through event system
		BroadcastMomentumChanged(CurrentMomentum, CurrentMomentum - OldMomentum);

		return true;
	}

	return false;
}

void UMomentumComponent::BankMomentum_Implementation(float Amount)
{
	// Check if we have enough momentum to bank
	if (CurrentMomentum >= Amount && BankedMomentum < MaxBankedMomentum)
	{
		// Calculate how much we can actually bank (limited by max banked)
		float AmountToBankActual = FMath::Min(Amount, MaxBankedMomentum - BankedMomentum);

		// Store old momentum for delta calculation
		float OldMomentum = CurrentMomentum;

		// Remove from current momentum
		CurrentMomentum -= AmountToBankActual;

		// Add to banked momentum
		BankedMomentum += AmountToBankActual;

		// Check for tier change
		int32 NewTier = GetMomentumTier_Implementation();
		if (NewTier != PreviousMomentumTier)
		{
			PreviousMomentumTier = NewTier;
			
			// Broadcast through delegate (legacy support)
			OnMomentumTierChanged.Broadcast(NewTier);
			
			// Broadcast through event system
			BroadcastMomentumTierChanged(NewTier);
		}

		// Broadcast momentum changed event
		OnMomentumChanged.Broadcast(CurrentMomentum, CurrentMomentum - OldMomentum);
		
		// Broadcast through event system
		BroadcastMomentumChanged(CurrentMomentum, CurrentMomentum - OldMomentum);
	}
}

float UMomentumComponent::WithdrawBankedMomentum_Implementation(float Amount)
{
	// Calculate how much we can actually withdraw
	float AmountToWithdraw = FMath::Min(Amount, BankedMomentum);

	// Remove from banked momentum
	BankedMomentum -= AmountToWithdraw;

	// Store old momentum for delta calculation
	float OldMomentum = CurrentMomentum;

	// Add to current momentum, capped at max
	CurrentMomentum = FMath::Min(CurrentMomentum + AmountToWithdraw, MaxMomentum);

	// Check for tier change
	int32 NewTier = GetMomentumTier_Implementation();
	if (NewTier != PreviousMomentumTier)
	{
		PreviousMomentumTier = NewTier;
		
		// Broadcast through delegate (legacy support)
		OnMomentumTierChanged.Broadcast(NewTier);
		
		// Broadcast through event system
		BroadcastMomentumTierChanged(NewTier);
	}

	// Broadcast momentum changed event
	OnMomentumChanged.Broadcast(CurrentMomentum, CurrentMomentum - OldMomentum);
	
	// Broadcast through event system
	BroadcastMomentumChanged(CurrentMomentum, CurrentMomentum - OldMomentum);

	return AmountToWithdraw;
}

int32 UMomentumComponent::GetMomentumTier_Implementation() const
{
	// Determine tier based on current momentum
	for (int32 i = MomentumThresholds.Num() - 1; i >= 0; --i)
	{
		if (CurrentMomentum >= MomentumThresholds[i])
		{
			return i + 1; // 1-based tier index (Tier 1, 2, 3, etc.)
		}
	}

	return 0; // Tier 0 if below all thresholds
}

float UMomentumComponent::GetMomentumRatio_Implementation() const
{
	// Return ratio of current to max momentum
	return FMath::Clamp(CurrentMomentum / MaxMomentum, 0.0f, 1.0f);
}

float UMomentumComponent::GetDamageMultiplier_Implementation() const
{
	// Base multiplier based on tier
	float BaseMultiplier = 1.0f;
	int32 Tier = GetMomentumTier_Implementation();

	// Each tier adds 10% damage
	BaseMultiplier += (Tier * 0.1f);

	// Apply overcharge multiplier if active
	if (bIsOvercharged)
	{
		BaseMultiplier *= OverchargeDamageMultiplier;
	}

	return BaseMultiplier;
}

void UMomentumComponent::OnTakeDamage(float DamageAmount)
{
	// Taking damage reduces momentum
	// The more damage taken, the more momentum lost
	float MomentumLoss = DamageAmount * 0.5f; // Adjust this multiplier as needed

	// Store old momentum for delta calculation
	float OldMomentum = CurrentMomentum;

	// Reduce momentum
	CurrentMomentum = FMath::Max(CurrentMomentum - MomentumLoss, 0.0f);

	// Check for tier change
	int32 NewTier = GetMomentumTier_Implementation();
	if (NewTier != PreviousMomentumTier)
	{
		PreviousMomentumTier = NewTier;
		
		// Broadcast through delegate (legacy support)
		OnMomentumTierChanged.Broadcast(NewTier);
		
		// Broadcast through event system
		BroadcastMomentumTierChanged(NewTier);
	}

	// Broadcast momentum changed event
	OnMomentumChanged.Broadcast(CurrentMomentum, CurrentMomentum - OldMomentum);
	
	// Broadcast through event system
	BroadcastMomentumChanged(CurrentMomentum, CurrentMomentum - OldMomentum);
}

bool UMomentumComponent::CanPerformMomentumAction_Implementation(FName ActionName, float RequiredMomentum) const
{
	// Check if we have enough momentum for the action
	return CurrentMomentum >= RequiredMomentum;
}

// Visual feedback methods

FLinearColor UMomentumComponent::GetMomentumColor() const
{
	// Define color gradient based on momentum tiers
	// Low momentum: Blue
	// Medium momentum: Purple
	// High momentum: Red
	// Overcharged: Gold/Yellow
	
	if (bIsOvercharged)
	{
		// Gold/Yellow for overcharge
		return FLinearColor(1.0f, 0.84f, 0.0f);
	}
	
	// Get normalized momentum value (0.0 to 1.0)
	float NormalizedMomentum = GetMomentumRatio_Implementation();
	
	// Define color stops
	FLinearColor LowColor = FLinearColor(0.0f, 0.5f, 1.0f); // Blue
	FLinearColor MidColor = FLinearColor(0.5f, 0.0f, 1.0f); // Purple
	FLinearColor HighColor = FLinearColor(1.0f, 0.0f, 0.0f); // Red
	
	// Interpolate between colors based on momentum
	if (NormalizedMomentum < 0.5f)
	{
		// Interpolate between low and mid
		float Alpha = NormalizedMomentum / 0.5f;
		return FMath::Lerp(LowColor, MidColor, Alpha);
	}
	else
	{
		// Interpolate between mid and high
		float Alpha = (NormalizedMomentum - 0.5f) / 0.5f;
		return FMath::Lerp(MidColor, HighColor, Alpha);
	}
}

float UMomentumComponent::GetMomentumIntensity() const
{
	// Return a value between 0.0 and 1.0 representing the intensity of momentum effects
	// This can be used for particle effects, sound, etc.
	
	// Get normalized momentum value
	float NormalizedMomentum = GetMomentumRatio_Implementation();
	
	// Apply a curve to make the intensity more pronounced at higher momentum levels
	float Intensity = FMath::Pow(NormalizedMomentum, 1.5f);
	
	// Boost intensity during overcharge
	if (bIsOvercharged)
	{
		Intensity = FMath::Min(1.0f, Intensity * 1.5f);
	}
	
	return Intensity;
}

void UMomentumComponent::TriggerMomentumPulse(float Intensity)
{
	// Check cooldown
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastMomentumPulseTime < MomentumPulseCooldown)
	{
		return;
	}
	
	// Update last pulse time
	LastMomentumPulseTime = CurrentTime;
	
	// Clamp intensity
	Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	
	// Broadcast through delegate
	OnMomentumPulseTriggered.Broadcast(Intensity);
	
	// Broadcast through event system if available
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::MomentumChanged; // Reuse momentum changed event type
		EventData.Instigator = GetOwner();
		EventData.FloatValue = Intensity;
		EventData.StringValue = FString("MomentumPulse"); // Use string to distinguish this as a pulse event
		
		// Broadcast the event
		EventSystem->BroadcastEvent(EventData);
	}
}

void UMomentumComponent::UpdateOverchargeState()
{
	// Check if we've entered or exited overcharge state
	bool NewOverchargeState = (CurrentMomentum >= MaxMomentum * OverchargeThreshold);

	if (NewOverchargeState != bWasOvercharged)
	{
		bIsOvercharged = NewOverchargeState;
		bWasOvercharged = NewOverchargeState;

		// Broadcast state change through delegate (legacy support)
		OnOverchargeStateChanged.Broadcast(bIsOvercharged);
		
		// Broadcast through event system
		BroadcastOverchargeStateChanged(bIsOvercharged);
	}
}

float UMomentumComponent::CalculateDiminishingReturns(FName SourceName, float BaseAmount)
{
	// Get current time
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Check if this source has been used recently
	if (MomentumSourceLastUseTimes.Contains(SourceName))
	{
		float LastUseTime = MomentumSourceLastUseTimes[SourceName];
		float TimeSinceLastUse = CurrentTime - LastUseTime;

		// If used within the last 2 seconds, apply diminishing returns
		if (TimeSinceLastUse < 2.0f)
		{
			// Calculate diminishing factor (0.5 to 1.0 based on time since last use)
			float DiminishingFactor = FMath::Lerp(0.5f, 1.0f, TimeSinceLastUse / 2.0f);
			return BaseAmount * DiminishingFactor;
		}
	}

	// No diminishing returns if not used recently
	return BaseAmount;
}

// Event handlers for game events
void UMomentumComponent::OnStylePointsGained(const FGameEventData& EventData)
{
	// Convert style points to momentum (at a reduced rate)
	float StylePoints = EventData.FloatValue;
	float MomentumGain = StylePoints * 0.2f;
	
	// Add momentum
	AddMomentum(MomentumGain, EventData.NameValue);
}

void UMomentumComponent::OnPlayerDamaged(const FGameEventData& EventData)
{
	// Handle player taking damage
	OnTakeDamage(EventData.FloatValue);
}

void UMomentumComponent::OnEnemyDefeated(const FGameEventData& EventData)
{
	// Add momentum for defeating an enemy
	AddMomentum(10.0f, FName("EnemyDefeated"));
}

void UMomentumComponent::OnWeaponFired(const FGameEventData& EventData)
{
	// Add a small amount of momentum for firing a weapon
	AddMomentum(2.0f, FName("WeaponFire"));
}

// Platforming-specific momentum gain methods

float UMomentumComponent::AddWallRunMomentum(float Duration)
{
	// Wall running gives continuous momentum based on duration
	// The longer the wall run, the more momentum gained, but with diminishing returns
	float MomentumGain = WallRunMomentumGain * FMath::Min(1.0f, Duration / 3.0f);
	
	// Add momentum
	AddMomentum(MomentumGain, MOMENTUM_SOURCE_WALL_RUN);
	
	// If this is a significant gain, trigger a pulse
	if (MomentumGain > WallRunMomentumGain * 0.5f)
	{
		TriggerMomentumPulse(MomentumGain / WallRunMomentumGain);
	}
	
	return MomentumGain;
}

float UMomentumComponent::AddDoubleJumpMomentum()
{
	// Double jump gives a fixed amount of momentum
	float MomentumGain = DoubleJumpMomentumGain;
	
	// Add momentum
	AddMomentum(MomentumGain, MOMENTUM_SOURCE_DOUBLE_JUMP);
	
	// Trigger a small pulse
	TriggerMomentumPulse(0.3f);
	
	return MomentumGain;
}

float UMomentumComponent::AddWallBounceMomentum()
{
	// Wall bounce gives a significant momentum boost
	float MomentumGain = WallBounceMomentumGain;
	
	// Add momentum
	AddMomentum(MomentumGain, MOMENTUM_SOURCE_WALL_BOUNCE);
	
	// Trigger a medium pulse
	TriggerMomentumPulse(0.6f);
	
	return MomentumGain;
}

float UMomentumComponent::AddRailGrindMomentum(float Duration)
{
	// Rail grinding gives continuous momentum based on duration
	// The longer the grind, the more momentum gained, but with diminishing returns
	float MomentumGain = RailGrindMomentumGain * FMath::Min(1.0f, Duration / 5.0f);
	
	// Add momentum
	AddMomentum(MomentumGain, MOMENTUM_SOURCE_RAIL_GRIND);
	
	// If this is a significant gain, trigger a pulse
	if (MomentumGain > RailGrindMomentumGain * 0.7f)
	{
		TriggerMomentumPulse(MomentumGain / RailGrindMomentumGain);
	}
	
	return MomentumGain;
}

float UMomentumComponent::AddRiftChainMomentum(int32 ChainCount)
{
	// Rift chaining gives increasing momentum based on chain count
	// Each chain in the sequence gives more momentum
	float ChainMultiplier = FMath::Min(3.0f, 1.0f + (ChainCount * 0.5f));
	float MomentumGain = RiftChainMomentumGain * ChainMultiplier;
	
	// Add momentum
	AddMomentum(MomentumGain, MOMENTUM_SOURCE_RIFT_CHAIN);
	
	// Trigger a pulse with intensity based on chain count
	TriggerMomentumPulse(FMath::Min(1.0f, ChainCount / 3.0f));
	
	return MomentumGain;
}

// Broadcast events through the event system
void UMomentumComponent::BroadcastMomentumChanged(float NewMomentum, float MomentumDelta)
{
	if (!EventSystem)
	{
		return;
	}
	
	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::MomentumChanged;
	EventData.Instigator = GetOwner();
	EventData.FloatValue = NewMomentum;
	EventData.BoolValue = (MomentumDelta > 0.0f); // True if momentum increased
	
	// Broadcast the event
	EventSystem->BroadcastEvent(EventData);
}

void UMomentumComponent::BroadcastMomentumTierChanged(int32 NewTier)
{
	if (!EventSystem)
	{
		return;
	}
	
	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::MomentumTierChanged;
	EventData.Instigator = GetOwner();
	EventData.IntValue = NewTier;
	
	// Broadcast the event
	EventSystem->BroadcastEvent(EventData);
}

void UMomentumComponent::BroadcastOverchargeStateChanged(bool bNewOverchargeState)
{
	if (!EventSystem)
	{
		return;
	}
	
	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::MomentumChanged; // Reuse momentum changed event type
	EventData.Instigator = GetOwner();
	EventData.FloatValue = CurrentMomentum;
	EventData.BoolValue = bNewOverchargeState;
	EventData.NameValue = FName("Overcharge"); // Use name to distinguish this as an overcharge event
	
	// Broadcast the event
	EventSystem->BroadcastEvent(EventData);
}

// Momentum preservation methods

void UMomentumComponent::PreserveMomentum(float Duration)
{
	// Cap the duration by the maximum allowed time
	Duration = FMath::Min(Duration, MaxMomentumPreservationTime);
	
	// If preservation is already active, extend the duration up to the max
	if (bMomentumPreservationActive)
	{
		float RemainingTime = CurrentMomentumPreservationDuration - (GetWorld()->GetTimeSeconds() - MomentumPreservationStartTime);
		CurrentMomentumPreservationDuration = FMath::Min(MaxMomentumPreservationTime, RemainingTime + Duration);
		
		// Update the timer
		GetWorld()->GetTimerManager().ClearTimer(MomentumPreservationTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			MomentumPreservationTimerHandle,
			this,
			&UMomentumComponent::OnMomentumPreservationTimerExpired,
			CurrentMomentumPreservationDuration,
			false
		);
	}
	else
	{
		// Start new preservation period
		bMomentumPreservationActive = true;
		MomentumPreservationStartTime = GetWorld()->GetTimeSeconds();
		CurrentMomentumPreservationDuration = Duration;
		
		// Set timer to end preservation
		GetWorld()->GetTimerManager().SetTimer(
			MomentumPreservationTimerHandle,
			this,
			&UMomentumComponent::OnMomentumPreservationTimerExpired,
			Duration,
			false
		);
		
		// Broadcast state change
		BroadcastMomentumPreservationChanged(true);
	}
}

void UMomentumComponent::ResumeMomentumDecay()
{
	// Only do something if preservation is active
	if (bMomentumPreservationActive)
	{
		// Clear the timer
		GetWorld()->GetTimerManager().ClearTimer(MomentumPreservationTimerHandle);
		
		// Update state
		bMomentumPreservationActive = false;
		
		// Broadcast state change
		BroadcastMomentumPreservationChanged(false);
	}
}

void UMomentumComponent::OnMomentumPreservationTimerExpired()
{
	// Update state
	bMomentumPreservationActive = false;
	
	// Broadcast state change
	BroadcastMomentumPreservationChanged(false);
}

// Platforming ability availability

bool UMomentumComponent::IsPlatformingAbilityAvailable(FName AbilityName) const
{
	// Check if the ability is on cooldown
	if (PlatformingAbilityCooldowns.Contains(AbilityName))
	{
		return false;
	}
	
	// Find the ability threshold data
	for (const FPlatformingAbilityThreshold& Ability : PlatformingAbilities)
	{
		if (Ability.AbilityName == AbilityName)
		{
			// Check if we meet the momentum requirement
			bool bHasEnoughMomentum = CurrentMomentum >= Ability.RequiredMomentum;
			
			// Check if we meet the tier requirement (if applicable)
			bool bMeetsTierRequirement = !Ability.bRequiresTier || GetMomentumTier_Implementation() >= Ability.RequiredTier;
			
			return bHasEnoughMomentum && bMeetsTierRequirement;
		}
	}
	
	// Ability not found
	return false;
}

void UMomentumComponent::UpdatePlatformingAbilityAvailability()
{
	// Check each ability for availability changes
	for (const FPlatformingAbilityThreshold& Ability : PlatformingAbilities)
	{
		bool bIsAvailable = IsPlatformingAbilityAvailable(Ability.AbilityName);
		bool bWasAvailable = false;
		
		// Check if we have a previous state for this ability
		if (PreviousAbilityAvailability.Contains(Ability.AbilityName))
		{
			bWasAvailable = PreviousAbilityAvailability[Ability.AbilityName];
		}
		
		// If availability changed, broadcast the event
		if (bIsAvailable != bWasAvailable)
		{
			PreviousAbilityAvailability.Add(Ability.AbilityName, bIsAvailable);
			BroadcastPlatformingAbilityAvailabilityChanged(Ability.AbilityName, bIsAvailable);
		}
	}
}

void UMomentumComponent::BroadcastPlatformingAbilityAvailabilityChanged(FName AbilityName, bool bIsAvailable)
{
	// Broadcast through delegate
	OnPlatformingAbilityAvailabilityChanged.Broadcast(AbilityName, bIsAvailable);
	
	// Broadcast through event system if available
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::MomentumChanged; // Reuse momentum changed event type
		EventData.Instigator = GetOwner();
		EventData.NameValue = AbilityName;
		EventData.BoolValue = bIsAvailable;
		EventData.StringValue = FString("PlatformingAbility"); // Use string to distinguish this as an ability event
		
		// Broadcast the event
		EventSystem->BroadcastEvent(EventData);
	}
}

void UMomentumComponent::BroadcastMomentumPreservationChanged(bool bIsActive)
{
	// Broadcast through delegate
	OnMomentumPreservationChanged.Broadcast(bIsActive);
	
	// Broadcast through event system if available
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::MomentumChanged; // Reuse momentum changed event type
		EventData.Instigator = GetOwner();
		EventData.BoolValue = bIsActive;
		EventData.StringValue = FString("MomentumPreservation"); // Use string to distinguish this as a preservation event
		
		// Broadcast the event
		EventSystem->BroadcastEvent(EventData);
	}
}