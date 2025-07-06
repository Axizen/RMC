// Fill out your copyright notice in the Description page of Project Settings.

#include "MomentumEffectsExample.h"

// Sets default values for this component's properties
UMomentumEffectsExample::UMomentumEffectsExample()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	OscillationMinValue = 0.0f;
	OscillationMaxValue = 100.0f;
	OscillationPeriod = 5.0f;
	bOscillationActive = false;
	OscillationTime = 0.0f;

	bCycleActive = false;
	CycleTime = 0.0f;
	CycleDuration = 10.0f;
}

// Called when the game starts
void UMomentumEffectsExample::BeginPlay()
{
	Super::BeginPlay();

	// Get the event system
	EventSystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameEventSubsystem>();
	if (!EventSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to get GameEventSubsystem. Events will not be broadcast."), *GetNameSafe(this));
	}
}

// Called when the component is being destroyed
void UMomentumEffectsExample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timers
	GetWorld()->GetTimerManager().ClearTimer(OscillationTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(CycleTimerHandle);

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UMomentumEffectsExample::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update oscillation if active
	if (bOscillationActive)
	{
		UpdateOscillation(DeltaTime);
	}

	// Update cycle if active
	if (bCycleActive)
	{
		UpdateCycle(DeltaTime);
	}
}

void UMomentumEffectsExample::SimulateMomentumChange(float MomentumValue)
{
	// Broadcast momentum changed event
	BroadcastMomentumChangedEvent(MomentumValue);
}

void UMomentumEffectsExample::SimulateMomentumTierChange(int32 NewTier)
{
	// Broadcast momentum tier changed event
	BroadcastMomentumTierChangedEvent(NewTier);
}

void UMomentumEffectsExample::SimulateOverchargeStateChange(bool bIsOvercharged)
{
	// Broadcast overcharge state changed event
	BroadcastOverchargeStateChangedEvent(bIsOvercharged);
}

void UMomentumEffectsExample::SimulateMomentumOscillation(float MinValue, float MaxValue, float Period, bool bAutoStart)
{
	// Store oscillation parameters
	OscillationMinValue = MinValue;
	OscillationMaxValue = MaxValue;
	OscillationPeriod = Period;
	OscillationTime = 0.0f;

	// Start oscillation if requested
	if (bAutoStart)
	{
		StartMomentumOscillation();
	}
}

void UMomentumEffectsExample::StartMomentumOscillation()
{
	// Start oscillation
	bOscillationActive = true;
	OscillationTime = 0.0f;

	// Broadcast initial momentum value
	BroadcastMomentumChangedEvent(OscillationMinValue);
}

void UMomentumEffectsExample::StopMomentumOscillation()
{
	// Stop oscillation
	bOscillationActive = false;
}

void UMomentumEffectsExample::SimulateCompleteMomentumCycle(float CycleDuration)
{
	// Store cycle parameters
	this->CycleDuration = CycleDuration;
	CycleTime = 0.0f;

	// Start cycle
	bCycleActive = true;

	// Broadcast initial momentum value
	BroadcastMomentumChangedEvent(0.0f);
	BroadcastMomentumTierChangedEvent(0);
	BroadcastOverchargeStateChangedEvent(false);
}

void UMomentumEffectsExample::UpdateOscillation(float DeltaTime)
{
	// Update oscillation time
	OscillationTime += DeltaTime;

	// Calculate normalized time (0.0 to 1.0)
	float NormalizedTime = FMath::Fmod(OscillationTime, OscillationPeriod) / OscillationPeriod;

	// Calculate momentum value using sine wave
	float Alpha = (FMath::Sin(NormalizedTime * 2.0f * PI) + 1.0f) * 0.5f; // 0.0 to 1.0
	float MomentumValue = FMath::Lerp(OscillationMinValue, OscillationMaxValue, Alpha);

	// Broadcast momentum changed event
	BroadcastMomentumChangedEvent(MomentumValue);

	// Determine momentum tier based on value
	int32 Tier = 0;
	if (MomentumValue >= 80.0f) Tier = 4;
	else if (MomentumValue >= 60.0f) Tier = 3;
	else if (MomentumValue >= 40.0f) Tier = 2;
	else if (MomentumValue >= 20.0f) Tier = 1;

	// Broadcast tier changed event if needed
	static int32 LastTier = -1;
	if (Tier != LastTier)
	{
		BroadcastMomentumTierChangedEvent(Tier);
		LastTier = Tier;
	}

	// Broadcast overcharge state changed event if needed
	static bool bLastOvercharged = false;
	bool bOvercharged = MomentumValue >= 95.0f;
	if (bOvercharged != bLastOvercharged)
	{
		BroadcastOverchargeStateChangedEvent(bOvercharged);
		bLastOvercharged = bOvercharged;
	}
}

void UMomentumEffectsExample::UpdateCycle(float DeltaTime)
{
	// Update cycle time
	CycleTime += DeltaTime;

	// Check if cycle is complete
	if (CycleTime >= CycleDuration)
	{
		// Reset cycle
		CycleTime = 0.0f;
		bCycleActive = false;

		// Broadcast final values
		BroadcastMomentumChangedEvent(0.0f);
		BroadcastMomentumTierChangedEvent(0);
		BroadcastOverchargeStateChangedEvent(false);

		return;
	}

	// Calculate normalized time (0.0 to 1.0)
	float NormalizedTime = CycleTime / CycleDuration;

	// Phase 1: Momentum builds up (0.0 to 0.4)
	if (NormalizedTime < 0.4f)
	{
		// Normalize to 0.0 to 1.0 for this phase
		float PhaseAlpha = NormalizedTime / 0.4f;
		
		// Calculate momentum value (0 to 80)
		float MomentumValue = PhaseAlpha * 80.0f;
		BroadcastMomentumChangedEvent(MomentumValue);

		// Determine momentum tier based on value
		int32 Tier = 0;
		if (MomentumValue >= 80.0f) Tier = 4;
		else if (MomentumValue >= 60.0f) Tier = 3;
		else if (MomentumValue >= 40.0f) Tier = 2;
		else if (MomentumValue >= 20.0f) Tier = 1;

		// Broadcast tier changed event if needed
		static int32 LastTier = -1;
		if (Tier != LastTier)
		{
			BroadcastMomentumTierChangedEvent(Tier);
			LastTier = Tier;
		}
	}
	// Phase 2: Overcharge builds up (0.4 to 0.6)
	else if (NormalizedTime < 0.6f)
	{
		// Normalize to 0.0 to 1.0 for this phase
		float PhaseAlpha = (NormalizedTime - 0.4f) / 0.2f;
		
		// Calculate momentum value (80 to 100)
		float MomentumValue = 80.0f + (PhaseAlpha * 20.0f);
		BroadcastMomentumChangedEvent(MomentumValue);

		// Broadcast tier changed event if needed
		static int32 LastTier = -1;
		if (LastTier != 4)
		{
			BroadcastMomentumTierChangedEvent(4);
			LastTier = 4;
		}

		// Broadcast overcharge state changed event if needed
		static bool bLastOvercharged = false;
		bool bOvercharged = MomentumValue >= 95.0f;
		if (bOvercharged != bLastOvercharged)
		{
			BroadcastOverchargeStateChangedEvent(bOvercharged);
			bLastOvercharged = bOvercharged;
		}
	}
	// Phase 3: Maintain overcharge (0.6 to 0.8)
	else if (NormalizedTime < 0.8f)
	{
		// Keep momentum at 100
		BroadcastMomentumChangedEvent(100.0f);

		// Ensure overcharge is active
		static bool bLastOvercharged = false;
		if (!bLastOvercharged)
		{
			BroadcastOverchargeStateChangedEvent(true);
			bLastOvercharged = true;
		}
	}
	// Phase 4: Momentum decays (0.8 to 1.0)
	else
	{
		// Normalize to 0.0 to 1.0 for this phase
		float PhaseAlpha = (NormalizedTime - 0.8f) / 0.2f;
		
		// Calculate momentum value (100 to 0)
		float MomentumValue = 100.0f * (1.0f - PhaseAlpha);
		BroadcastMomentumChangedEvent(MomentumValue);

		// Determine momentum tier based on value
		int32 Tier = 0;
		if (MomentumValue >= 80.0f) Tier = 4;
		else if (MomentumValue >= 60.0f) Tier = 3;
		else if (MomentumValue >= 40.0f) Tier = 2;
		else if (MomentumValue >= 20.0f) Tier = 1;

		// Broadcast tier changed event if needed
		static int32 LastTier = 4;
		if (Tier != LastTier)
		{
			BroadcastMomentumTierChangedEvent(Tier);
			LastTier = Tier;
		}

		// Broadcast overcharge state changed event if needed
		static bool bLastOvercharged = true;
		bool bOvercharged = MomentumValue >= 95.0f;
		if (bOvercharged != bLastOvercharged)
		{
			BroadcastOverchargeStateChangedEvent(bOvercharged);
			bLastOvercharged = bOvercharged;
		}
	}
}

void UMomentumEffectsExample::BroadcastMomentumChangedEvent(float MomentumValue)
{
	if (!EventSystem)
	{
		return;
	}

	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::MomentumChanged;
	EventData.Instigator = GetOwner();
	EventData.FloatValue = MomentumValue;

	// Broadcast event
	EventSystem->BroadcastEvent(EventData);
}

void UMomentumEffectsExample::BroadcastMomentumTierChangedEvent(int32 NewTier)
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

	// Broadcast event
	EventSystem->BroadcastEvent(EventData);
}

void UMomentumEffectsExample::BroadcastOverchargeStateChangedEvent(bool bIsOvercharged)
{
	if (!EventSystem)
	{
		return;
	}

	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::OverchargeStateChanged;
	EventData.Instigator = GetOwner();
	EventData.BoolValue = bIsOvercharged;

	// Broadcast event
	EventSystem->BroadcastEvent(EventData);
}