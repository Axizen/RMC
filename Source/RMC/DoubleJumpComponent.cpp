// Fill out your copyright notice in the Description page of Project Settings.

#include "DoubleJumpComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/ComponentLocator.h"
#include "Core/MomentumInterface.h"
#include "Core/GameEventSystem.h"
#include "MomentumComponent.h"

// Sets default values for this component's properties
UDoubleJumpComponent::UDoubleJumpComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize event listener handles array
	EventListenerHandles.Empty();
}

// Register with ComponentLocator
void UDoubleJumpComponent::OnRegister()
{
	Super::OnRegister();

	// Register with the component locator
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->RegisterComponent(this, FString("DoubleJumpComponent"));
		}
	}
}

// Unregister from ComponentLocator
void UDoubleJumpComponent::OnUnregister()
{
	// Unregister from the component locator
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->UnregisterComponent(FString("DoubleJumpComponent"));
		}
	}

	Super::OnUnregister();
}

// Clean up event listeners
void UDoubleJumpComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove all event listeners
	if (EventSystem)
	{
		for (int32 Handle : EventListenerHandles)
		{
			EventSystem->RemoveEventListener(Handle);
		}
		EventListenerHandles.Empty();
	}

	Super::EndPlay(EndPlayReason);
}

// Called when the game starts
void UDoubleJumpComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get references to required components
	AActor* Owner = GetOwner();
	if (Owner)
	{
		ACharacter* Character = Cast<ACharacter>(Owner);
		if (Character)
		{
			CharacterMovement = Character->GetCharacterMovement();
			CapsuleComponent = Character->GetCapsuleComponent();
		}
	}

	// Get the event system
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance)
	{
		EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
		if (EventSystem)
		{
			// Register for momentum changed events
			int32 MomentumHandle = EventSystem->AddEventListener(this, EGameEventType::MomentumChanged, FName("OnMomentumChanged"));
			EventListenerHandles.Add(MomentumHandle);

			// Register for landed events (custom event we'll need to broadcast from the character)
			// We'll define a new event type for this
			// For now, we'll use a workaround in the tick function to detect landing
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: Failed to get GameEventSubsystem. Event-based communication will be disabled."), *GetName());
		}
	}

	// Check if we have a momentum interface
	if (!GetMomentumInterface())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No MomentumInterface found on owner. Double jump momentum integration will be disabled."), *GetName());
	}
}

// Called every frame
void UDoubleJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check if we've landed on the ground
	if (CharacterMovement && CharacterMovement->IsMovingOnGround() && CurrentDoubleJumpCount > 0)
	{
		// Reset double jump count when landing
		ResetDoubleJumpCountInternal();
	}

	// Update double jump visuals if we're in a double jump
	if (bIsDoubleJumping)
	{
		UpdateDoubleJumpVisuals();

		// Check if we're still in the air
		if (CharacterMovement && CharacterMovement->IsMovingOnGround())
		{
			// We've landed, so we're no longer double jumping
			bIsDoubleJumping = false;
		}
	}
}

bool UDoubleJumpComponent::CanDoubleJumpInternal() const
{
	// Check if required components exist
	if (!CharacterMovement || !CapsuleComponent)
	{
		return false;
	}

	// Check if in cooldown
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastDoubleJumpTime < DoubleJumpCooldown)
	{
		return false;
	}

	// Check if in air
	if (CharacterMovement->IsMovingOnGround())
	{
		return false;
	}

	// Check if we've used all our double jumps
	if (CurrentDoubleJumpCount >= GetMaxDoubleJumpsWithMomentum())
	{
		return false;
	}

	// Check if we have enough momentum to double jump
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		// Get the UMomentumComponent directly
		UMomentumComponent* MomentumComp = GetMomentumComponent();
		if (MomentumComp)
		{
			// Check if the DoubleJump ability is available
			if (!MomentumComp->IsPlatformingAbilityAvailable(FName("DoubleJump")))
			{
				// Double jump ability not available
				return false;
			}
		}
		else
		{
			// Fallback to interface method
			const float RequiredMomentum = MinMomentumForDoubleJump;
			if (MomentumInterface->GetCurrentMomentum() < RequiredMomentum)
			{
				// Not enough momentum to double jump
				return false;
			}
		}
	}

	return true;
}

bool UDoubleJumpComponent::PerformDoubleJumpInternal()
{
	// Check if we can double jump
	if (!CanDoubleJumpInternal())
	{
		return false;
	}

	// Get character
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !CharacterMovement)
	{
		return false;
	}

	// Calculate jump force with momentum bonus
	float JumpForce = GetDoubleJumpForceWithMomentum();

	// Apply jump velocity
	FVector JumpVelocity = FVector(0.0f, 0.0f, JumpForce);

	// Add horizontal boost in the direction of movement
	FVector HorizontalVelocity = CharacterMovement->Velocity;
	HorizontalVelocity.Z = 0.0f;

	if (!HorizontalVelocity.IsNearlyZero())
	{
		HorizontalVelocity.Normalize();
		JumpVelocity += HorizontalVelocity * DoubleJumpHorizontalBoost;
	}

	// Apply velocity
	CharacterMovement->Velocity = FVector(CharacterMovement->Velocity.X, CharacterMovement->Velocity.Y, 0.0f);
	CharacterMovement->Velocity += JumpVelocity;
	CharacterMovement->SetMovementMode(MOVE_Falling);

	// Increment double jump count
	CurrentDoubleJumpCount++;

	// Set double jumping flag
	bIsDoubleJumping = true;

	// Store jump time for cooldown
	LastDoubleJumpTime = GetWorld()->GetTimeSeconds();

	// Add momentum for double jumping
	HandleDoubleJumpMomentum();

	// Update visuals
	UpdateDoubleJumpVisuals();

	// Broadcast event
	BroadcastDoubleJumpEvent();

	// Broadcast legacy event
	OnDoubleJumpPerformed.Broadcast();

	return true;
}

void UDoubleJumpComponent::ResetDoubleJumpCountInternal()
{
	// Reset double jump count
	CurrentDoubleJumpCount = 0;

	// Reset double jumping flag
	bIsDoubleJumping = false;

	// Broadcast event
	BroadcastDoubleJumpResetEvent();

	// Broadcast legacy event
	OnDoubleJumpReset.Broadcast();
}

float UDoubleJumpComponent::GetDoubleJumpForceWithMomentum() const
{
	// Base jump force
	float JumpForce = DoubleJumpForce;

	// Apply momentum-based bonus
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		// Get the UMomentumComponent directly
		UMomentumComponent* MomentumComp = GetMomentumComponent();
		if (MomentumComp)
		{
			// Get momentum ratio (0-1) and apply scaled bonus
			float MomentumRatio = MomentumComp->GetMomentumRatio();
			JumpForce += DoubleJumpForce * MomentumRatio * MomentumScaledJumpBoost;

			// Apply tier-based bonuses
			int32 MomentumTier = MomentumComp->GetMomentumTier();
			if (MomentumTier >= 1)
			{
				JumpForce += DoubleJumpForce * MomentumTier1JumpBonus;
			}
			if (MomentumTier >= 2)
			{
				JumpForce += DoubleJumpForce * MomentumTier2JumpBonus;
			}
			if (MomentumTier >= 3)
			{
				JumpForce += DoubleJumpForce * MomentumTier3JumpBonus;
			}
		}
		else
		{
			// Fallback to interface method
			float MomentumRatio = MomentumInterface->GetMomentumRatio();
			JumpForce += DoubleJumpForce * MomentumRatio * MomentumScaledJumpBoost;
		}
	}

	return JumpForce;
}

int32 UDoubleJumpComponent::GetMaxDoubleJumpsWithMomentum() const
{
	// Base max double jumps
	int32 MaxJumps = MaxDoubleJumps;

	// Apply momentum-based bonus
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		// Get the UMomentumComponent directly
		UMomentumComponent* MomentumComp = GetMomentumComponent();
		if (MomentumComp)
		{
			// Apply tier-based bonuses
			int32 MomentumTier = MomentumComp->GetMomentumTier();
			if (MomentumTier >= 2)
			{
				// Tier 2 grants an extra double jump
				MaxJumps += 1;
			}
			if (MomentumTier >= 3)
			{
				// Tier 3 grants another extra double jump
				MaxJumps += 1;
			}
		}
		else
		{
			// Fallback to interface method
			int32 MomentumTier = MomentumInterface->GetMomentumTier();
			if (MomentumTier >= 2)
			{
				MaxJumps += 1;
			}
			if (MomentumTier >= 3)
			{
				MaxJumps += 1;
			}
		}
	}

	return MaxJumps;
}

void UDoubleJumpComponent::HandleDoubleJumpMomentum()
{
	// Add momentum for double jumping using interface
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (!MomentumInterface)
	{
		return;
	}

	// Get the UMomentumComponent directly
	UMomentumComponent* MomentumComp = GetMomentumComponent();
	if (MomentumComp)
	{
		// Use the specialized double jump momentum method
		MomentumComp->AddDoubleJumpMomentum();
		
		// Preserve momentum during the double jump
		MomentumComp->PreserveMomentum(0.5f);
		
		// Trigger a momentum pulse for visual feedback
		MomentumComp->TriggerMomentumPulse(0.6f);
	}
	else
	{
		// Fallback to standard interface method
		MomentumInterface->AddMomentum(DoubleJumpMomentumGain, MOMENTUM_SOURCE_DOUBLE_JUMP);
	}
}

void UDoubleJumpComponent::UpdateDoubleJumpVisuals()
{
	// Get momentum interface
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (!MomentumInterface)
	{
		return;
	}

	// Get the UMomentumComponent directly
	UMomentumComponent* MomentumComp = GetMomentumComponent();
	if (!MomentumComp)
	{
		return;
	}

	// Get momentum color and intensity for visual effects
	FLinearColor MomentumColor = MomentumComp->GetMomentumColor();
	float MomentumIntensity = MomentumComp->GetMomentumIntensity();

	// Here you would apply these values to any visual effects
	// For example, if you have a trail effect component:
	// if (TrailComponent)
	// {
	//     TrailComponent->SetColorParameter("Color", MomentumColor);
	//     TrailComponent->SetFloatParameter("Intensity", MomentumIntensity);
	// }

	// You could also spawn a particle effect at the character's feet
	// UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DoubleJumpEffect, GetOwner()->GetActorLocation(), FRotator::ZeroRotator, FVector(MomentumIntensity));
}

void UDoubleJumpComponent::BroadcastDoubleJumpEvent()
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::WallRunStarted; // Reusing this event type for now, we should add a DoubleJumpPerformed event type
		EventData.Instigator = GetOwner();
		EventData.LocationValue = GetOwner()->GetActorLocation();
		EventData.NameValue = FName("DoubleJump");
		EventData.IntValue = CurrentDoubleJumpCount;
		
		EventSystem->BroadcastEvent(EventData);
	}
}

void UDoubleJumpComponent::BroadcastDoubleJumpResetEvent()
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::WallRunEnded; // Reusing this event type for now, we should add a DoubleJumpReset event type
		EventData.Instigator = GetOwner();
		EventData.LocationValue = GetOwner()->GetActorLocation();
		EventData.NameValue = FName("DoubleJumpReset");
		
		EventSystem->BroadcastEvent(EventData);
	}
}

void UDoubleJumpComponent::ResetDoubleJumpCooldown()
{
	// Clear cooldown timer
	GetWorld()->GetTimerManager().ClearTimer(DoubleJumpCooldownTimer);
}

// Event handlers
void UDoubleJumpComponent::OnMomentumChanged(const FGameEventData& EventData)
{
	// React to momentum changes
	if (bIsDoubleJumping)
	{
		// Update visuals when momentum changes during double jumping
		UpdateDoubleJumpVisuals();
	}

	// Check if max double jumps has changed due to momentum tier change
	int32 NewMaxDoubleJumps = GetMaxDoubleJumpsWithMomentum();
	static int32 PreviousMaxDoubleJumps = MaxDoubleJumps;

	if (NewMaxDoubleJumps != PreviousMaxDoubleJumps)
	{
		// Broadcast max double jumps changed event
		OnMaxDoubleJumpsChanged.Broadcast(NewMaxDoubleJumps);
		PreviousMaxDoubleJumps = NewMaxDoubleJumps;
	}
}

void UDoubleJumpComponent::OnLanded(const FGameEventData& EventData)
{
	// Reset double jump count when landing
	ResetDoubleJumpCountInternal();
}

// DoubleJumpInterface implementation
bool UDoubleJumpComponent::CanDoubleJump_Implementation() const
{
	return CanDoubleJumpInternal();
}

bool UDoubleJumpComponent::PerformDoubleJump_Implementation()
{
	return PerformDoubleJumpInternal();
}

int32 UDoubleJumpComponent::GetDoubleJumpCount_Implementation() const
{
	return CurrentDoubleJumpCount;
}

int32 UDoubleJumpComponent::GetMaxDoubleJumps_Implementation() const
{
	return GetMaxDoubleJumpsWithMomentum();
}

float UDoubleJumpComponent::GetDoubleJumpForce_Implementation() const
{
	return GetDoubleJumpForceWithMomentum();
}

void UDoubleJumpComponent::ResetDoubleJumpCount_Implementation()
{
	ResetDoubleJumpCountInternal();
}

bool UDoubleJumpComponent::IsDoubleJumping_Implementation() const
{
	return bIsDoubleJumping;
}

// Interface helpers
IMomentumInterface* UDoubleJumpComponent::GetMomentumInterface() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}
	
	// Check if the owner implements the interface
	if (Owner->Implements<UMomentumInterface>())
	{
		return Cast<IMomentumInterface>(Owner);
	}
	
	// If not, try to find a component that implements it
	UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		UActorComponent* Component = Locator->GetComponentByClass(UMomentumInterface::StaticClass());
		if (Component && Component->Implements<UMomentumInterface>())
		{
			return Cast<IMomentumInterface>(Component);
		}
	}
	
	// As a fallback, try to find any component that implements the interface
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component && Component->Implements<UMomentumInterface>())
		{
			return Cast<IMomentumInterface>(Component);
		}
	}
	
	return nullptr;
}

UMomentumComponent* UDoubleJumpComponent::GetMomentumComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Try to find the component directly
	UMomentumComponent* MomentumComp = Owner->FindComponentByClass<UMomentumComponent>();
	if (MomentumComp)
	{
		return MomentumComp;
	}

	// If not found directly, try to find it through the component locator
	UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		UActorComponent* Component = Locator->GetComponentByClass(UMomentumComponent::StaticClass());
		return Cast<UMomentumComponent>(Component);
	}

	return nullptr;
}