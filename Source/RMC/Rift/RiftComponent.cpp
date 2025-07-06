// Fill out your copyright notice in the Description page of Project Settings.

#include "RiftComponent.h"
#include "RiftAnchor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Core/GameEventSystem.h"
#include "../Core/ComponentLocator.h"

// Sets default values for this component's properties
URiftComponent::URiftComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	CurrentRiftState = ERiftState::Idle;
	RiftCooldown = 0.5f;
	RiftMomentumGain = 10.0f;
	RemainingCooldown = 0.0f;
	CurrentChainCount = 0;
	CurrentAnchor = nullptr;
	ComponentLocator = nullptr;
	EventSystem = nullptr;

	// Counter rift properties
	CounterRiftWindow = 0.5f;
	CounterRiftDistance = 150.0f;
	CounterRiftMomentumGain = 15.0f;
	CounterRiftStylePoints = 200.0f;
	bInCounterRiftWindow = false;
	LastAttacker = nullptr;

	// Default base capabilities
	BaseCapabilities.MaxRiftDistance = 500.0f;
	BaseCapabilities.MaxChainCount = 1;
	BaseCapabilities.PhantomDodgeDuration = 0.3f;
	BaseCapabilities.PhantomDodgeDistance = 300.0f;
	BaseCapabilities.bCanAerialReset = false;
	BaseCapabilities.bCanCounterRift = false;

	// Initialize current capabilities to base values
	CurrentCapabilities = BaseCapabilities;
}

void URiftComponent::OnRegister()
{
	Super::OnRegister();

	// Register with component locator if available
	AActor* Owner = GetOwner();
	if (Owner)
	{
		ComponentLocator = Owner->FindComponentByClass<UComponentLocator>();
		if (ComponentLocator)
		{
			ComponentLocator->RegisterComponent(this, FName("RiftComponent"));
		}
	}
}

void URiftComponent::OnUnregister()
{
	// Unregister from component locator
	if (ComponentLocator)
	{
		ComponentLocator->UnregisterComponent(this);
		ComponentLocator = nullptr;
	}

	Super::OnUnregister();
}

void URiftComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
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
void URiftComponent::BeginPlay()
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

		// Get reference to the event system
		UGameInstance* GameInstance = GetWorld()->GetGameInstance();
		if (GameInstance)
		{
			EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
			if (EventSystem)
			{
				// Register for events
				int32 Handle = EventSystem->AddEventListenerWithObject(EGameEventType::PlayerDamaged, this, FName("OnPlayerDamaged"));
				EventListenerHandles.Add(Handle);
			}
		}
	}

	// Initialize capabilities
	UpdateRiftCapabilities();
}

// Called every frame
void URiftComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update capabilities based on current momentum
	UpdateRiftCapabilities();
}

// IRiftInterface implementation
bool URiftComponent::CanRift_Implementation() const
{
	return CurrentRiftState == ERiftState::Idle;
}

ERiftState URiftComponent::GetRiftState_Implementation() const
{
	return CurrentRiftState;
}

FRiftCapabilities URiftComponent::GetRiftCapabilities_Implementation() const
{
	return CurrentCapabilities;
}

bool URiftComponent::PerformPhantomDodge_Implementation(FVector Direction, bool bIsAerial)
{
	return PerformPhantomDodge(Direction, bIsAerial);
}

bool URiftComponent::InitiateRiftTether_Implementation(const FVector& TargetLocation)
{
	return InitiateRiftTether(TargetLocation);
}

bool URiftComponent::ChainRiftTether_Implementation()
{
	return ChainRiftTether();
}

// Helper methods for accessing interfaces
IMomentumInterface* URiftComponent::GetMomentumInterface() const
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<UMomentumInterface>())
	{
		return Cast<IMomentumInterface>(Owner);
	}

	// Try to find through component locator
	if (ComponentLocator)
	{
		UActorComponent* Component = ComponentLocator->GetComponentByName(FName("MomentumComponent"));
		if (Component && Component->Implements<UMomentumInterface>())
		{
			return Cast<IMomentumInterface>(Component);
		}
	}

	return nullptr;
}

IStyleInterface* URiftComponent::GetStyleInterface() const
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<UStyleInterface>())
	{
		return Cast<IStyleInterface>(Owner);
	}

	// Try to find through component locator
	if (ComponentLocator)
	{
		UActorComponent* Component = ComponentLocator->GetComponentByName(FName("StyleComponent"));
		if (Component && Component->Implements<UStyleInterface>())
		{
			return Cast<IStyleInterface>(Component);
		}
	}

	return nullptr;
}

// Event handlers
void URiftComponent::OnPlayerDamaged(const FGameEventData& EventData)
{
	// Only process if this is our owner
	if (EventData.Target == GetOwner())
	{
		NotifyTakeDamage(EventData.FloatValue, EventData.Instigator);
	}
}

// Broadcast events
void URiftComponent::BroadcastRiftPerformedEvent(FName RiftType, const FVector& StartLocation, const FVector& EndLocation)
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::RiftPerformed;
		EventData.Instigator = GetOwner();
		EventData.NameValue = RiftType;
		EventData.LocationValue = StartLocation;
		EventData.RotationValue.Vector() = EndLocation; // Using rotation to store end location

		EventSystem->BroadcastEvent(EventData);
	}
}

void URiftComponent::BroadcastRiftStateChangedEvent(ERiftState NewState, ERiftState OldState)
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::RiftStateChanged;
		EventData.Instigator = GetOwner();
		EventData.IntValue = static_cast<int32>(NewState);
		EventData.FloatValue = static_cast<float>(OldState); // Using float to store old state

		EventSystem->BroadcastEvent(EventData);
	}
}

bool URiftComponent::PerformPhantomDodge(FVector Direction, bool bIsAerial)
{
	// Check if we can perform a dodge
	if (CurrentRiftState != ERiftState::Idle || !CharacterMovement)
	{
		return false;
	}

	// Check if aerial reset is allowed
	if (bIsAerial && !CurrentCapabilities.bCanAerialReset)
	{
		return false;
	}

	// Normalize direction
	if (!Direction.IsNearlyZero())
	{
		Direction.Normalize();
	}
	else
	{
		// Use forward vector if no direction specified
		Direction = GetOwner()->GetActorForwardVector();
	}

	// Calculate target location
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = CurrentLocation + (Direction * CurrentCapabilities.PhantomDodgeDistance);

	// Check if target location is valid
	if (!IsValidRiftLocation(TargetLocation))
	{
		// Try to find a valid location along the path
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());

		if (GetWorld()->LineTraceSingleByChannel(HitResult, CurrentLocation, TargetLocation, ECC_Visibility, QueryParams))
		{
			// Use hit location minus some offset to avoid embedding in surfaces
			TargetLocation = HitResult.Location - (Direction * 50.0f);
		}
	}

	// Store old state for event
	ERiftState OldState = CurrentRiftState;

	// Set current state to dodging
	CurrentRiftState = ERiftState::Dodging;
	CurrentRiftTarget = TargetLocation;

	// Broadcast rift state changed event
	BroadcastRiftStateChangedEvent(CurrentRiftState, OldState);

	// Broadcast rift begin event (legacy)
	OnRiftBegin.Broadcast();

	// Execute the teleport
	ExecuteRiftTeleport(TargetLocation);

	// If aerial, reset jump count
	if (bIsAerial && CharacterMovement)
	{
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character)
		{
			Character->JumpCurrentCount = 0;
			CharacterMovement->SetMovementMode(MOVE_Falling);
		}
	}

	// Handle momentum effects
	HandleRiftMomentumEffects(true);

	// Broadcast phantom dodge event
	FName DodgeType = bIsAerial ? FName("AerialPhantomDodge") : FName("PhantomDodge");
	BroadcastRiftPerformedEvent(DodgeType, CurrentLocation, TargetLocation);

	// Set timer to complete the rift
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &URiftComponent::CompleteRift, true);
	GetWorld()->GetTimerManager().SetTimer(
		RiftCompletionTimerHandle,
		TimerDelegate,
		CurrentCapabilities.PhantomDodgeDuration,
		false);

	return true;
}

bool URiftComponent::InitiateRiftTether(const FVector& TargetLocation)
{
	// Check if we can perform a tether
	if (CurrentRiftState != ERiftState::Idle)
	{
		return false;
	}

	// Check if target is within range
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	float Distance = FVector::Distance(CurrentLocation, TargetLocation);

	if (Distance > CurrentCapabilities.MaxRiftDistance)
	{
		return false;
	}

	// Check if target location is valid
	if (!IsValidRiftLocation(TargetLocation))
	{
		return false;
	}

	// Store old state for event
	ERiftState OldState = CurrentRiftState;

	// Set current state to tethering
	CurrentRiftState = ERiftState::Tethering;
	CurrentRiftTarget = TargetLocation;
	CurrentChainCount = 0;

	// Broadcast rift state changed event
	BroadcastRiftStateChangedEvent(CurrentRiftState, OldState);

	// Broadcast rift begin event (legacy)
	OnRiftBegin.Broadcast();

	// Execute the teleport
	ExecuteRiftTeleport(TargetLocation);

	// Handle momentum effects
	HandleRiftMomentumEffects(true);

	// Broadcast rift tether event
	BroadcastRiftPerformedEvent(FName("RiftTether"), CurrentLocation, TargetLocation);

	// Set timer to complete the rift
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &URiftComponent::CompleteRift, true);
	GetWorld()->GetTimerManager().SetTimer(
		RiftCompletionTimerHandle,
		TimerDelegate,
		0.2f, // Short duration for tether completion
		false);

	return true;
}

bool URiftComponent::InitiateRiftTetherToAnchor(ARiftAnchor* TargetAnchor)
{
	// Check if anchor is valid
	if (!TargetAnchor)
	{
		return false;
	}

	// Check if we can perform a tether
	if (CurrentRiftState != ERiftState::Idle)
	{
		return false;
	}

	// Check if anchor can be used with current momentum
	float CurrentMomentumValue = 0.0f;
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		CurrentMomentumValue = MomentumInterface->Execute_GetCurrentMomentum(Cast<UObject>(MomentumInterface));
	}

	if (!TargetAnchor->CanUseWithMomentum(CurrentMomentumValue))
	{
		return false;
	}

	// Check if target is within range
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = TargetAnchor->GetActorLocation();
	float Distance = FVector::Distance(CurrentLocation, TargetLocation);

	if (Distance > CurrentCapabilities.MaxRiftDistance)
	{
		return false;
	}

	// Store old state for event
	ERiftState OldState = CurrentRiftState;

	// Set current state to tethering
	CurrentRiftState = ERiftState::Tethering;
	CurrentRiftTarget = TargetLocation;
	CurrentChainCount = 0;

	// Store reference to the current anchor for chaining
	CurrentAnchor = TargetAnchor;

	// Broadcast rift state changed event
	BroadcastRiftStateChangedEvent(CurrentRiftState, OldState);

	// Broadcast rift begin event (legacy)
	OnRiftBegin.Broadcast();

	// Execute the teleport
	ExecuteRiftTeleport(TargetLocation);

	// Notify the anchor
	TargetAnchor->OnRiftTo();

	// Handle momentum effects
	HandleRiftMomentumEffects(true);

	// Set timer to complete the rift
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &URiftComponent::CompleteRift, true);
	GetWorld()->GetTimerManager().SetTimer(
		RiftCompletionTimerHandle,
		TimerDelegate,
		0.2f, // Short duration for tether completion
		false);

	return true;
}

bool URiftComponent::ChainRiftTether()
{
	// Check if we can chain a tether
	if (CurrentRiftState != ERiftState::Tethering)
	{
		return false;
	}

	// Check if we've reached the chain limit
	if (CurrentChainCount >= CurrentCapabilities.MaxChainCount)
	{
		return false;
	}

	// Check if we have a current anchor
	if (!CurrentAnchor)
	{
		return false;
	}

	// Get the next anchor in the chain
	ARiftAnchor* NextAnchor = CurrentAnchor->GetNextAnchor();
	
	// Get current momentum value from interface
	float CurrentMomentumValue = 0.0f;
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		CurrentMomentumValue = MomentumInterface->Execute_GetCurrentMomentum(Cast<UObject>(MomentumInterface));
	}

	if (!NextAnchor || !NextAnchor->CanUseWithMomentum(CurrentMomentumValue))
	{
		return false;
	}

	// Check if next anchor is within range
	FVector CurrentLocation = GetOwner()->GetActorLocation();
	FVector NextLocation = NextAnchor->GetActorLocation();
	float Distance = FVector::Distance(CurrentLocation, NextLocation);

	if (Distance > CurrentCapabilities.MaxRiftDistance * 1.5f) // Allow slightly longer distance for chains
	{
		return false;
	}

	// Update current target
	CurrentRiftTarget = NextLocation;
	CurrentAnchor = NextAnchor;

	// Execute the teleport
	ExecuteRiftTeleport(NextLocation);

	// Notify the anchor
	NextAnchor->OnRiftTo();

	// Handle momentum effects
	HandleRiftMomentumEffects(true);

	// Increment chain count
	CurrentChainCount++;

	// Broadcast chain rift event
	BroadcastRiftPerformedEvent(FName("ChainRift"), CurrentLocation, NextLocation);

	// Set timer to complete the rift
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &URiftComponent::CompleteRift, true);
	GetWorld()->GetTimerManager().SetTimer(
		RiftCompletionTimerHandle,
		TimerDelegate,
		0.2f, // Short duration for tether completion
		false);

	return true;
}

bool URiftComponent::AttemptCounterRift()
{
	// Check if counter rift is available
	if (!CurrentCapabilities.bCanCounterRift || CurrentRiftState != ERiftState::Idle)
	{
		return false;
	}

	// Check if we're in the counter rift window and have a valid attacker
	if (!bInCounterRiftWindow || !LastAttacker || !IsValid(LastAttacker))
	{
		return false;
	}

	// Calculate position behind the attacker
	FVector AttackerLocation = LastAttacker->GetActorLocation();
	FVector AttackerForward = LastAttacker->GetActorForwardVector();
	FVector CounterPosition = AttackerLocation - (AttackerForward * CounterRiftDistance);

	// Check if the counter position is valid
	if (!IsValidRiftLocation(CounterPosition))
	{
		// Try to find a valid position nearby
		FVector OwnerLocation = GetOwner()->GetActorLocation();
		FVector DirectionToAttacker = (AttackerLocation - OwnerLocation).GetSafeNormal();
		
		// Try positions at different angles around the attacker
		for (int32 i = 0; i < 8; i++)
		{
			// Rotate around in 45-degree increments
			float Angle = FMath::DegreesToRadians(i * 45.0f);
			FVector RotatedDir = DirectionToAttacker.RotateAngleAxis(Angle, FVector(0, 0, 1));
			FVector TestPosition = AttackerLocation + (RotatedDir * CounterRiftDistance);
			
			if (IsValidRiftLocation(TestPosition))
			{
				CounterPosition = TestPosition;
				break;
			}
		}
		
		// If we still don't have a valid position, fail
		if (!IsValidRiftLocation(CounterPosition))
		{
			return false;
		}
	}

	// Store old state for event
	ERiftState OldState = CurrentRiftState;

	// Set current state to dodging (using the dodge state for counter rift)
	CurrentRiftState = ERiftState::Dodging;
	CurrentRiftTarget = CounterPosition;

	// Broadcast rift state changed event
	BroadcastRiftStateChangedEvent(CurrentRiftState, OldState);

	// Broadcast rift begin event (legacy)
	OnRiftBegin.Broadcast();

	// Execute the teleport
	ExecuteRiftTeleport(CounterPosition);

	// Make the owner face the attacker after teleporting
	if (GetOwner())
	{
		FVector DirectionToAttacker = (AttackerLocation - CounterPosition).GetSafeNormal();
		FRotator NewRotation = DirectionToAttacker.Rotation();
		GetOwner()->SetActorRotation(NewRotation);
	}

	// Apply bonus momentum for successful counter rift
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		MomentumInterface->Execute_AddMomentum(Cast<UObject>(MomentumInterface), CounterRiftMomentumGain, FName("CounterRift"));
	}

	// Add style points if available
	IStyleInterface* StyleInterface = GetStyleInterface();
	if (StyleInterface)
	{
		StyleInterface->Execute_AddStylePoints(Cast<UObject>(StyleInterface), CounterRiftStylePoints, FName("CounterRift"));
	}

	// Broadcast counter rift performed event (legacy)
	OnCounterRiftPerformed.Broadcast(LastAttacker);

	// Broadcast counter rift event
	FVector StartLocation = GetOwner()->GetActorLocation();
	BroadcastRiftPerformedEvent(FName("CounterRift"), StartLocation, CounterPosition);

	// Clear counter rift window
	bInCounterRiftWindow = false;
	GetWorld()->GetTimerManager().ClearTimer(CounterRiftWindowTimerHandle);

	// Set timer to complete the rift
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &URiftComponent::CompleteRift, true);
	GetWorld()->GetTimerManager().SetTimer(
		RiftCompletionTimerHandle,
		TimerDelegate,
		CurrentCapabilities.PhantomDodgeDuration,
		false);

	// Reset last attacker
	LastAttacker = nullptr;

	return true;
}

void URiftComponent::UpdateRiftCapabilities()
{
	// Get momentum factor from MomentumInterface if available
	float MomentumFactor = 0.0f;
	IMomentumInterface* MomentumInterface = GetMomentumInterface();
	if (MomentumInterface)
	{
		MomentumFactor = MomentumInterface->Execute_GetMomentumRatio(Cast<UObject>(MomentumInterface));
	}

	// Update capabilities based on momentum
	CurrentCapabilities.MaxRiftDistance = BaseCapabilities.MaxRiftDistance * (1.0f + MomentumFactor);
	CurrentCapabilities.PhantomDodgeDistance = BaseCapabilities.PhantomDodgeDistance * (1.0f + MomentumFactor * 0.5f);
	CurrentCapabilities.PhantomDodgeDuration = BaseCapabilities.PhantomDodgeDuration * (1.0f + MomentumFactor * 0.3f);

	// Update chain count based on momentum thresholds
	if (MomentumFactor >= 0.5f)
	{
		CurrentCapabilities.MaxChainCount = 2;
	}
	else
	{
		CurrentCapabilities.MaxChainCount = BaseCapabilities.MaxChainCount;
	}

	// Enable aerial reset at 30% momentum
	CurrentCapabilities.bCanAerialReset = (MomentumFactor >= 0.3f) || BaseCapabilities.bCanAerialReset;

	// Enable counter rift at 70% momentum
	CurrentCapabilities.bCanCounterRift = (MomentumFactor >= 0.7f) || BaseCapabilities.bCanCounterRift;
}

void URiftComponent::HandleRiftMomentumEffects(bool bSuccessfulRift)
{
	if (bSuccessfulRift)
	{
		// Gain momentum for successful rift using MomentumInterface
		IMomentumInterface* MomentumInterface = GetMomentumInterface();
		if (MomentumInterface)
		{
			MomentumInterface->Execute_AddMomentum(Cast<UObject>(MomentumInterface), RiftMomentumGain, FName("Rift"));
		}

		// Apply momentum-based movement effects
		if (CharacterMovement)
		{
			// Boost velocity based on momentum
			float MomentumFactor = 0.0f;
			if (MomentumInterface)
			{
				MomentumFactor = MomentumInterface->Execute_GetMomentumRatio(Cast<UObject>(MomentumInterface));
			}

			FVector CurrentVelocity = CharacterMovement->Velocity;
			FVector ForwardDir = GetOwner()->GetActorForwardVector();

			// Apply a forward boost proportional to momentum
			CharacterMovement->Velocity = CurrentVelocity + (ForwardDir * 300.0f * MomentumFactor);
		}

		// Broadcast rift performed event
		FVector StartLocation = GetOwner()->GetActorLocation() - (GetOwner()->GetActorForwardVector() * 100.0f); // Approximate start location
		FVector EndLocation = GetOwner()->GetActorLocation();
		BroadcastRiftPerformedEvent(FName("Rift"), StartLocation, EndLocation);
	}
}

void URiftComponent::CompleteRift(bool bSuccessful)
{
	// Clear the timer
	GetWorld()->GetTimerManager().ClearTimer(RiftCompletionTimerHandle);

	// Store old state for event
	ERiftState OldState = CurrentRiftState;

	// Broadcast rift end event (legacy)
	OnRiftEnd.Broadcast();

	// Increment chain count if successful
	if (bSuccessful && CurrentRiftState == ERiftState::Tethering)
	{
		CurrentChainCount++;
	}

	// Start cooldown if we're not chaining or if we've reached the chain limit
	if (!bSuccessful || CurrentRiftState == ERiftState::Dodging || 
		CurrentChainCount >= CurrentCapabilities.MaxChainCount)
	{
		StartCooldown();
	}
	else
	{
		// Return to tethering state to allow for chaining
		ERiftState NewState = ERiftState::Tethering;
		CurrentRiftState = NewState;
		
		// Broadcast state change event
		BroadcastRiftStateChangedEvent(NewState, OldState);
	}
}

void URiftComponent::StartCooldown()
{
	// Store old state for event
	ERiftState OldState = CurrentRiftState;

	// Set state to cooldown
	CurrentRiftState = ERiftState::Cooldown;
	RemainingCooldown = RiftCooldown;

	// Broadcast state change event
	BroadcastRiftStateChangedEvent(CurrentRiftState, OldState);

	// Reset current anchor
	CurrentAnchor = nullptr;

	// Set timer for cooldown
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&URiftComponent::OnCooldownEnd,
		RiftCooldown,
		false);
}

void URiftComponent::OnCooldownEnd()
{
	// Clear the timer
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerHandle);

	// Store old state for event
	ERiftState OldState = CurrentRiftState;

	// Reset state
	CurrentRiftState = ERiftState::Idle;
	RemainingCooldown = 0.0f;
	CurrentChainCount = 0;

	// Broadcast state change event
	BroadcastRiftStateChangedEvent(CurrentRiftState, OldState);
}

void URiftComponent::ExecuteRiftTeleport(const FVector& TargetLocation)
{
	// Perform the actual teleportation
	AActor* Owner = GetOwner();
	if (Owner)
	{
		// Save current rotation
		FRotator CurrentRotation = Owner->GetActorRotation();

		// Teleport the actor
		Owner->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

		// Maintain rotation
		Owner->SetActorRotation(CurrentRotation);

		// If we have a character movement component, adjust velocity
		if (CharacterMovement)
		{
			// Preserve horizontal velocity but reset vertical velocity
			FVector NewVelocity = CharacterMovement->Velocity;
			NewVelocity.Z = 0.0f;
			CharacterMovement->Velocity = NewVelocity;
		}
	}
}

bool URiftComponent::IsValidRiftLocation(const FVector& TargetLocation) const
{
	// Check if there's enough space at the target location
	if (!CapsuleComponent || !GetWorld())
	{
		return false;
	}

	// Perform a capsule overlap check at the target location
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(GetOwner());

	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
		CapsuleComponent->GetScaledCapsuleRadius(),
		CapsuleComponent->GetScaledCapsuleHalfHeight());

	// Check for blocking hits
	bool bHasBlockingHit = GetWorld()->OverlapAnyTestByProfile(
		TargetLocation,
		FQuat::Identity,
		FName("Pawn"),
		CapsuleShape,
		FCollisionQueryParams(FName("RiftLocationCheck"), false, GetOwner()));

	return !bHasBlockingHit;
}

void URiftComponent::NotifyTakeDamage(float DamageAmount, AActor* DamageCauser)
{
	// Store the attacker for potential counter rift
	if (DamageCauser && DamageCauser != GetOwner())
	{
		LastAttacker = DamageCauser;
		
		// Only start counter window if we have the capability
		if (CurrentCapabilities.bCanCounterRift && CurrentRiftState == ERiftState::Idle)
		{
			// Start counter rift window
			bInCounterRiftWindow = true;
			
			// Set timer for counter rift window
			GetWorld()->GetTimerManager().SetTimer(
				CounterRiftWindowTimerHandle,
				this,
				&URiftComponent::OnCounterRiftWindowEnd,
				CounterRiftWindow,
				false);
			
			// Broadcast counter rift window opened event
			if (EventSystem)
			{
				FGameEventData EventData;
				EventData.EventType = EGameEventType::RiftStateChanged;
				EventData.Instigator = GetOwner();
				EventData.Target = DamageCauser;
				EventData.NameValue = FName("CounterRiftWindowOpened");
				EventData.FloatValue = CounterRiftWindow;
				
				EventSystem->BroadcastEvent(EventData);
			}
		}
	}
}

void URiftComponent::OnCounterRiftWindowEnd()
{
	// Clear the timer
	GetWorld()->GetTimerManager().ClearTimer(CounterRiftWindowTimerHandle);
	
	// Reset counter rift state
	bInCounterRiftWindow = false;
	
	// Broadcast counter rift window closed event
	if (EventSystem && LastAttacker)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::RiftStateChanged;
		EventData.Instigator = GetOwner();
		EventData.Target = LastAttacker;
		EventData.NameValue = FName("CounterRiftWindowClosed");
		
		EventSystem->BroadcastEvent(EventData);
	}
	
	LastAttacker = nullptr;
}