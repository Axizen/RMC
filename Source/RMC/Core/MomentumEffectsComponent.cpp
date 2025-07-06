// Fill out your copyright notice in the Description page of Project Settings.

#include "MomentumEffectsComponent.h"
#include "ComponentLocator.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UMomentumEffectsComponent::UMomentumEffectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	MinEffectIntensity = 0.0f;
	MaxEffectIntensity = 1.0f;
	EffectIntensityMultiplier = 1.0f;
	CurrentEffectIntensity = 0.0f;
	CurrentMomentumTier = 0;
	bIsOvercharged = false;
	MomentumLoopAudioComponent = nullptr;
	MomentumTrailComponent = nullptr;
	OverchargeComponent = nullptr;
}

// Called when the component is registered
void UMomentumEffectsComponent::OnRegister()
{
	Super::OnRegister();

	// Register with the component locator
	UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		Locator->RegisterComponent(this, TEXT("MomentumEffectsComponent"));
	}
}

// Called when the component is unregistered
void UMomentumEffectsComponent::OnUnregister()
{
	// Unregister from the component locator
	UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		Locator->UnregisterComponent(TEXT("MomentumEffectsComponent"));
	}

	Super::OnUnregister();
}

// Called when the game starts
void UMomentumEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get the event system
	EventSystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameEventSubsystem>();
	if (EventSystem)
	{
		// Register for momentum changed events
		FDelegateHandle MomentumChangedHandle = EventSystem->AddEventListener(
			this,
			EGameEventType::MomentumChanged,
			FName("OnMomentumChanged")
		);
		EventListenerHandles.Add(MomentumChangedHandle);

		// Register for momentum tier changed events
		FDelegateHandle MomentumTierChangedHandle = EventSystem->AddEventListener(
			this,
			EGameEventType::MomentumTierChanged,
			FName("OnMomentumTierChanged")
		);
		EventListenerHandles.Add(MomentumTierChangedHandle);

		// Register for overcharge state changed events
		FDelegateHandle OverchargeStateChangedHandle = EventSystem->AddEventListener(
			this,
			EGameEventType::OverchargeStateChanged,
			FName("OnOverchargeStateChanged")
		);
		EventListenerHandles.Add(OverchargeStateChangedHandle);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to get GameEventSubsystem. Effects will not respond to momentum changes."), *GetNameSafe(this));
	}

	// Create the momentum loop sound
	if (MomentumLoopSound)
	{
		MomentumLoopAudioComponent = UGameplayStatics::SpawnSoundAttached(
			MomentumLoopSound,
			GetOwner()->GetRootComponent(),
			FName("MomentumLoop"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true,
			0.0f,
			1.0f,
			0.0f
		);

		if (MomentumLoopAudioComponent)
		{
			MomentumLoopAudioComponent->SetVolumeMultiplier(0.0f);
			MomentumLoopAudioComponent->SetPitchMultiplier(1.0f);
		}
	}

	// Create the momentum trail effect
	if (MomentumTrailEffect)
	{
		MomentumTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			MomentumTrailEffect,
			GetOwner()->GetRootComponent(),
			FName("MomentumTrail"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true,
			true
		);

		if (MomentumTrailComponent)
		{
			MomentumTrailComponent->SetVariableFloat(FName("Intensity"), 0.0f);
		}
	}
}

// Called when the component is being destroyed
void UMomentumEffectsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up event listeners
	if (EventSystem)
	{
		for (FDelegateHandle Handle : EventListenerHandles)
		{
			EventSystem->RemoveEventListener(Handle);
		}
		EventListenerHandles.Empty();
	}

	// Clean up effects
	CleanupEffects();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UMomentumEffectsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update effect parameters based on current state
	if (MomentumLoopAudioComponent)
	{
		// Smoothly interpolate volume to target
		float TargetVolume = CurrentEffectIntensity * EffectIntensityMultiplier;
		float CurrentVolume = MomentumLoopAudioComponent->VolumeMultiplier;
		float NewVolume = FMath::FInterpTo(CurrentVolume, TargetVolume, DeltaTime, 5.0f);
		MomentumLoopAudioComponent->SetVolumeMultiplier(NewVolume);

		// Adjust pitch based on momentum tier
		float TargetPitch = 1.0f + (CurrentMomentumTier * 0.1f); // 10% pitch increase per tier
		float CurrentPitch = MomentumLoopAudioComponent->PitchMultiplier;
		float NewPitch = FMath::FInterpTo(CurrentPitch, TargetPitch, DeltaTime, 3.0f);
		MomentumLoopAudioComponent->SetPitchMultiplier(NewPitch);
	}

	if (MomentumTrailComponent)
	{
		// Smoothly interpolate intensity to target
		float TargetIntensity = CurrentEffectIntensity * EffectIntensityMultiplier;
		MomentumTrailComponent->SetVariableFloat(FName("Intensity"), TargetIntensity);
	}
}

// Event handler for momentum changed events
void UMomentumEffectsComponent::OnMomentumChanged(const FGameEventData& EventData)
{
	// Extract momentum value from event data
	float MomentumValue = EventData.FloatValue;

	// Update effect intensity based on momentum value
	UpdateEffectIntensity(MomentumValue);
}

// Event handler for momentum tier changed events
void UMomentumEffectsComponent::OnMomentumTierChanged(const FGameEventData& EventData)
{
	// Extract new tier from event data
	int32 NewTier = EventData.IntValue;

	// Determine if this is a tier up or down
	bool bTierUp = NewTier > CurrentMomentumTier;

	// Play appropriate tier change effect
	PlayTierChangeEffect(NewTier, bTierUp);

	// Update current tier
	CurrentMomentumTier = NewTier;
}

// Event handler for overcharge state changed events
void UMomentumEffectsComponent::OnOverchargeStateChanged(const FGameEventData& EventData)
{
	// Extract new overcharge state from event data
	bool bNewOverchargeState = EventData.BoolValue;

	// Update overcharge effect
	UpdateOverchargeEffect(bNewOverchargeState);

	// Update current overcharge state
	bIsOvercharged = bNewOverchargeState;
}

// Helper function to update effect intensity based on momentum value
void UMomentumEffectsComponent::UpdateEffectIntensity(float MomentumValue)
{
	// Calculate normalized intensity (0.0 to 1.0)
	float NormalizedIntensity = FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, 100.0f), // Assuming max momentum is 100
		FVector2D(MinEffectIntensity, MaxEffectIntensity),
		MomentumValue
	);

	// Update current intensity
	CurrentEffectIntensity = NormalizedIntensity;
}

// Helper function to play tier change effects
void UMomentumEffectsComponent::PlayTierChangeEffect(int32 NewTier, bool bTierUp)
{
	// Play appropriate sound effect
	USoundBase* SoundToPlay = bTierUp ? TierUpSound : TierDownSound;
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			SoundToPlay,
			GetOwner()->GetActorLocation(),
			1.0f,
			1.0f + (NewTier * 0.1f) // Pitch increases with tier
		);
	}

	// Play appropriate visual effect
	UNiagaraSystem* EffectToPlay = bTierUp ? TierUpEffect : TierDownEffect;
	if (EffectToPlay)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			EffectToPlay,
			GetOwner()->GetActorLocation(),
			GetOwner()->GetActorRotation(),
			FVector(1.0f + (NewTier * 0.2f)) // Scale increases with tier
		);
	}
}

// Helper function to update overcharge effect
void UMomentumEffectsComponent::UpdateOverchargeEffect(bool bNewOverchargeState)
{
	// Handle overcharge sound
	if (bNewOverchargeState && OverchargeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			OverchargeSound,
			GetOwner()->GetActorLocation()
		);
	}

	// Handle overcharge visual effect
	if (bNewOverchargeState && OverchargeEffect)
	{
		// Create overcharge effect if it doesn't exist
		if (!OverchargeComponent)
		{
			OverchargeComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				OverchargeEffect,
				GetOwner()->GetRootComponent(),
				FName("OverchargeEffect"),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true,
				true
			);
		}
	}
	else if (!bNewOverchargeState && OverchargeComponent)
	{
		// Destroy overcharge effect
		OverchargeComponent->DeactivateImmediate();
		OverchargeComponent->DestroyComponent();
		OverchargeComponent = nullptr;
	}

	// Increase effect intensity multiplier during overcharge
	EffectIntensityMultiplier = bNewOverchargeState ? 2.0f : 1.0f;
}

// Helper function to clean up effects
void UMomentumEffectsComponent::CleanupEffects()
{
	// Clean up audio components
	if (MomentumLoopAudioComponent)
	{
		MomentumLoopAudioComponent->Stop();
		MomentumLoopAudioComponent->DestroyComponent();
		MomentumLoopAudioComponent = nullptr;
	}

	// Clean up visual effects
	if (MomentumTrailComponent)
	{
		MomentumTrailComponent->DeactivateImmediate();
		MomentumTrailComponent->DestroyComponent();
		MomentumTrailComponent = nullptr;
	}

	if (OverchargeComponent)
	{
		OverchargeComponent->DeactivateImmediate();
		OverchargeComponent->DestroyComponent();
		OverchargeComponent = nullptr;
	}
}