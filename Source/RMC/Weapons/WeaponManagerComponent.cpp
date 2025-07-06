// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponManagerComponent.h"
#include "RangedWeaponBase.h"
#include "../Rift/RiftComponent.h"
#include "../Core/ComponentLocator.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UWeaponManagerComponent::UWeaponManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	CurrentWeaponIndex = 0;
	CurrentWeapon = nullptr;
	MaxWeapons = 8;
	EventSystem = nullptr;
}

// Called when the component is registered
void UWeaponManagerComponent::OnRegister()
{
	Super::OnRegister();

	// Register with the component locator if available
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
		if (Locator)
		{
			Locator->RegisterComponent(this, FName("WeaponManager"));
		}
	}
}

// Called when the component is unregistered
void UWeaponManagerComponent::OnUnregister()
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
void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get reference to the game event subsystem
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	if (GameInstance)
	{
		EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
	}

	// Register for events if we have an event system
	if (EventSystem)
	{
		// Register for rift state changes
		int32 Handle = EventSystem->AddEventListenerWithObject(
			EGameEventType::RiftStateChanged,
			this,
			FName("OnRiftStateChanged"));
		EventListenerHandles.Add(Handle);
	}

	// Legacy support: Get reference to owner's rift component
	URiftComponent* RiftComp = GetOwnerRiftComponent();

	// If we have a rift component, bind to its events for backward compatibility
	if (RiftComp)
	{
		RiftComp->OnRiftBegin.AddDynamic(this, &UWeaponManagerComponent::OnRiftBegin);
		RiftComp->OnRiftEnd.AddDynamic(this, &UWeaponManagerComponent::OnRiftEnd);
	}

	// Initialize weapons
	for (URangedWeaponBase* Weapon : WeaponInventory)
	{
		if (Weapon)
		{
			// Ensure the weapon is initialized
			// Note: ActorComponents don't have SetOwner, they're already owned by their outer
			Weapon->RegisterComponent();
		}
	}

	// Set initial weapon if we have any
	if (WeaponInventory.Num() > 0)
	{
		SwitchToWeapon(0);
	}
}

// Called when the game ends
void UWeaponManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up event listeners
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

// Called every frame
void UWeaponManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWeaponManagerComponent::SwitchToWeapon(int32 WeaponIndex)
{
	// Validate index
	if (WeaponIndex < 0 || WeaponIndex >= WeaponInventory.Num() || WeaponInventory[WeaponIndex] == nullptr)
	{
		return;
	}

	// Store previous weapon for the event
	URangedWeaponBase* PreviousWeapon = CurrentWeapon;

	// Update current weapon
	CurrentWeaponIndex = WeaponIndex;
	CurrentWeapon = WeaponInventory[WeaponIndex];

	// Broadcast weapon switched event through both systems
	OnWeaponSwitched.Broadcast(CurrentWeapon, PreviousWeapon);
	BroadcastWeaponSwitchedEvent(CurrentWeapon, PreviousWeapon);
}

void UWeaponManagerComponent::NextWeapon()
{
	// If we have no weapons, do nothing
	if (WeaponInventory.Num() == 0)
	{
		return;
	}

	// Calculate next index with wraparound
	int32 NextIndex = (CurrentWeaponIndex + 1) % WeaponInventory.Num();
	SwitchToWeapon(NextIndex);
}

void UWeaponManagerComponent::PreviousWeapon()
{
	// If we have no weapons, do nothing
	if (WeaponInventory.Num() == 0)
	{
		return;
	}

	// Calculate previous index with wraparound
	int32 PrevIndex = (CurrentWeaponIndex - 1 + WeaponInventory.Num()) % WeaponInventory.Num();
	SwitchToWeapon(PrevIndex);
}

bool UWeaponManagerComponent::AddWeaponToInventory(TSubclassOf<URangedWeaponBase> WeaponClass)
{
	// Check if we're at max capacity
	if (WeaponInventory.Num() >= MaxWeapons)
	{
		return false;
	}

	// Check if the weapon class is valid
	if (!WeaponClass)
	{
		return false;
	}

	// Check if we already have this weapon type
	for (URangedWeaponBase* ExistingWeapon : WeaponInventory)
	{
		if (ExistingWeapon && ExistingWeapon->GetClass() == WeaponClass)
		{
			// We already have this weapon type
			// Could implement ammo pickup logic here instead
			return false;
		}
	}

	// Create the new weapon
	URangedWeaponBase* NewWeapon = NewObject<URangedWeaponBase>(GetOwner(), WeaponClass);
	if (NewWeapon)
	{
		// Initialize the weapon
		// Note: ActorComponents don't have SetOwner, they're already owned by their outer
		NewWeapon->RegisterComponent();

		// Add to inventory
		WeaponInventory.Add(NewWeapon);

		// If this is our first weapon, switch to it
		if (WeaponInventory.Num() == 1)
		{
			SwitchToWeapon(0);
		}

		// Broadcast weapon added event through delegate
		OnWeaponAdded.Broadcast(NewWeapon);

		// Broadcast through event system
		if (EventSystem)
		{
			FGameEventData EventData;
			EventData.EventType = EGameEventType::WeaponFired; // Reuse weapon fired event type
			EventData.Instigator = GetOwner();
			EventData.NameValue = FName("WeaponAdded");
			EventData.IntValue = WeaponInventory.Num(); // Pass inventory count
			
			EventSystem->BroadcastEvent(EventData);
		}

		return true;
	}

	return false;
}

bool UWeaponManagerComponent::RemoveWeaponFromInventory(int32 WeaponIndex)
{
	// Validate index
	if (WeaponIndex < 0 || WeaponIndex >= WeaponInventory.Num())
	{
		return false;
	}

	// Get the weapon to remove
	URangedWeaponBase* WeaponToRemove = WeaponInventory[WeaponIndex];
	if (!WeaponToRemove)
	{
		return false;
	}

	// Remove from inventory
	WeaponInventory.RemoveAt(WeaponIndex);

	// If we removed the current weapon, switch to another one
	if (CurrentWeaponIndex == WeaponIndex)
	{
		if (WeaponInventory.Num() > 0)
		{
			// Switch to the first weapon
			SwitchToWeapon(0);
		}
		else
		{
			// No weapons left
			CurrentWeapon = nullptr;
			CurrentWeaponIndex = 0;
		}
	}
	// If we removed a weapon before the current one, adjust the index
	else if (CurrentWeaponIndex > WeaponIndex)
	{
		CurrentWeaponIndex--;
	}

	// Broadcast weapon removed event through delegate
	OnWeaponRemoved.Broadcast(WeaponToRemove);

	// Broadcast through event system
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::WeaponFired; // Reuse weapon fired event type
		EventData.Instigator = GetOwner();
		EventData.NameValue = FName("WeaponRemoved");
		EventData.IntValue = WeaponInventory.Num(); // Pass inventory count
		
		EventSystem->BroadcastEvent(EventData);
	}

	// Clean up the removed weapon
	WeaponToRemove->DestroyComponent();

	return true;
}

bool UWeaponManagerComponent::HasWeapon(TSubclassOf<URangedWeaponBase> WeaponClass) const
{
	for (URangedWeaponBase* Weapon : WeaponInventory)
	{
		if (Weapon && Weapon->GetClass() == WeaponClass)
		{
			return true;
		}
	}
	return false;
}

void UWeaponManagerComponent::FireCurrentWeapon()
{
	if (CurrentWeapon)
	{
		bool bSuccess = CurrentWeapon->FireWeapon();
		
		// Broadcast weapon fired event
		BroadcastWeaponFiredEvent(CurrentWeapon, bSuccess);
		
		// If successfully fired, add momentum through the interface
		if (bSuccess)
		{
			// Try to get momentum interface
			TScriptInterface<IMomentumInterface> MomentumInterface = GetMomentumInterface();
			if (MomentumInterface)
			{
				// Add a small amount of momentum for firing a weapon
				IMomentumInterface::Execute_AddMomentum(MomentumInterface.GetObject(), 2.0f, FName("WeaponFire"));
			}
			else
			{
				// Legacy support
				UMomentumComponent* MomentumComp = GetOwnerMomentumComponent();
				if (MomentumComp)
				{
					// Add a small amount of momentum for firing a weapon
					MomentumComp->AddMomentum(2.0f, FName("WeaponFire"));
				}
			}
		}
	}
}

void UWeaponManagerComponent::AltFireCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->AltFireWeapon();
	}
}

void UWeaponManagerComponent::StartReloadCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartReload();
		
		// Broadcast weapon reloaded event
		BroadcastWeaponReloadedEvent(CurrentWeapon);
	}
}

void UWeaponManagerComponent::StartChargeCurrentWeapon()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->AltFireWeapon(); // Start charging
	}
}

void UWeaponManagerComponent::ReleaseChargeCurrentWeapon()
{
	if (CurrentWeapon)
	{
		// Store the current charge level before releasing
		float ChargeLevel = CurrentWeapon->CurrentCharge;
		
		// Release the charge
		CurrentWeapon->ReleaseCharge();
		
		// If we had a significant charge, add momentum through the interface
		if (ChargeLevel > 0.2f)
		{
			// Try to get momentum interface
			TScriptInterface<IMomentumInterface> MomentumInterface = GetMomentumInterface();
			if (MomentumInterface)
			{
				// Add momentum proportional to charge level
				IMomentumInterface::Execute_AddMomentum(MomentumInterface.GetObject(), 5.0f * ChargeLevel, FName("ChargedShot"));
			}
			else
			{
				// Legacy support
				UMomentumComponent* MomentumComp = GetOwnerMomentumComponent();
				if (MomentumComp)
				{
					// Add momentum proportional to charge level
					MomentumComp->AddMomentum(5.0f * ChargeLevel, FName("ChargedShot"));
				}
			}
			
			// Broadcast weapon fired event with charge info
			FGameEventData EventData;
			EventData.EventType = EGameEventType::WeaponFired;
			EventData.Instigator = GetOwner();
			EventData.FloatValue = ChargeLevel; // Pass charge level as float value
			EventData.NameValue = FName("ChargedShot");
			EventData.BoolValue = true; // Indicate this was a charged shot
			
			if (EventSystem)
			{
				EventSystem->BroadcastEvent(EventData);
			}
		}
	}
}

void UWeaponManagerComponent::OnRiftStateChanged(const FGameEventData& EventData)
{
	// Check if this is a rift begin or end event
	if (EventData.NameValue == FName("Begin"))
	{
		// Notify current weapon of rift begin
		if (CurrentWeapon)
		{
			CurrentWeapon->OnRiftBegin();
		}
	}
	else if (EventData.NameValue == FName("End"))
	{
		// Notify current weapon of rift end
		if (CurrentWeapon)
		{
			CurrentWeapon->OnRiftEnd();
		}
	}
}

// Legacy support for direct rift component events
void UWeaponManagerComponent::OnRiftBegin()
{
	// Notify current weapon of rift begin
	if (CurrentWeapon)
	{
		CurrentWeapon->OnRiftBegin();
	}
}

void UWeaponManagerComponent::OnRiftEnd()
{
	// Notify current weapon of rift end
	if (CurrentWeapon)
	{
		CurrentWeapon->OnRiftEnd();
	}
}

// Broadcast weapon fired event
void UWeaponManagerComponent::BroadcastWeaponFiredEvent(URangedWeaponBase* Weapon, bool bSuccess)
{
	if (!EventSystem || !Weapon || !bSuccess)
	{
		return;
	}

	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::WeaponFired;
	EventData.Instigator = GetOwner();
	EventData.FloatValue = Weapon->BaseDamage; // Pass base damage as float value
	EventData.NameValue = FName("WeaponFire");
	EventData.BoolValue = bSuccess;

	// Broadcast the event
	EventSystem->BroadcastEvent(EventData);
}

// Broadcast weapon switched event
void UWeaponManagerComponent::BroadcastWeaponSwitchedEvent(URangedWeaponBase* NewWeapon, URangedWeaponBase* PreviousWeapon)
{
	if (!EventSystem || !NewWeapon)
	{
		return;
	}

	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::WeaponFired; // Reuse weapon fired event type
	EventData.Instigator = GetOwner();
	EventData.NameValue = FName("WeaponSwitch");

	// Broadcast the event
	EventSystem->BroadcastEvent(EventData);
}

// Broadcast weapon reloaded event
void UWeaponManagerComponent::BroadcastWeaponReloadedEvent(URangedWeaponBase* Weapon)
{
	if (!EventSystem || !Weapon)
	{
		return;
	}

	// Create event data
	FGameEventData EventData;
	EventData.EventType = EGameEventType::WeaponReloaded;
	EventData.Instigator = GetOwner();
	EventData.NameValue = FName("WeaponReload");

	// Broadcast the event
	EventSystem->BroadcastEvent(EventData);
}

// Helper functions to get interfaces
TScriptInterface<IMomentumInterface> UWeaponManagerComponent::GetMomentumInterface() const
{
	TScriptInterface<IMomentumInterface> Result;
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return Result;
	}

	// First try to get from component locator
	UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		UActorComponent* Component = Locator->GetComponentByName(FName("Momentum"));
		if (Component && Component->Implements<UMomentumInterface>())
		{
			Result.SetObject(Component);
			Result.SetInterface(Cast<IMomentumInterface>(Component));
			return Result;
		}
	}

	// If not found in locator, check if owner implements the interface
	if (Owner->Implements<UMomentumInterface>())
	{
		Result.SetObject(Owner);
		Result.SetInterface(Cast<IMomentumInterface>(Owner));
		return Result;
	}

	// If still not found, look for components that implement the interface
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component && Component->Implements<UMomentumInterface>())
		{
			Result.SetObject(Component);
			Result.SetInterface(Cast<IMomentumInterface>(Component));
			return Result;
		}
	}

	return Result;
}

TScriptInterface<IRiftInterface> UWeaponManagerComponent::GetRiftInterface() const
{
	TScriptInterface<IRiftInterface> Result;
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return Result;
	}

	// First try to get from component locator
	UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		UActorComponent* Component = Locator->GetComponentByName(FName("Rift"));
		if (Component && Component->Implements<URiftInterface>())
		{
			Result.SetObject(Component);
			Result.SetInterface(Cast<IRiftInterface>(Component));
			return Result;
		}
	}

	// If not found in locator, check if owner implements the interface
	if (Owner->Implements<URiftInterface>())
	{
		Result.SetObject(Owner);
		Result.SetInterface(Cast<IRiftInterface>(Owner));
		return Result;
	}

	// If still not found, look for components that implement the interface
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component && Component->Implements<URiftInterface>())
		{
			Result.SetObject(Component);
			Result.SetInterface(Cast<IRiftInterface>(Component));
			return Result;
		}
	}

	return Result;
}

TScriptInterface<IStyleInterface> UWeaponManagerComponent::GetStyleInterface() const
{
	TScriptInterface<IStyleInterface> Result;
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return Result;
	}

	// First try to get from component locator
	UComponentLocator* Locator = Owner->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		UActorComponent* Component = Locator->GetComponentByName(FName("Style"));
		if (Component && Component->Implements<UStyleInterface>())
		{
			Result.SetObject(Component);
			Result.SetInterface(Cast<IStyleInterface>(Component));
			return Result;
		}
	}

	// If not found in locator, check if owner implements the interface
	if (Owner->Implements<UStyleInterface>())
	{
		Result.SetObject(Owner);
		Result.SetInterface(Cast<IStyleInterface>(Owner));
		return Result;
	}

	// If still not found, look for components that implement the interface
	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (Component && Component->Implements<UStyleInterface>())
		{
			Result.SetObject(Component);
			Result.SetInterface(Cast<IStyleInterface>(Component));
			return Result;
		}
	}

	return Result;
}

// Legacy helper functions (for backward compatibility)
URiftComponent* UWeaponManagerComponent::GetOwnerRiftComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<URiftComponent>();
}

UMomentumComponent* UWeaponManagerComponent::GetOwnerMomentumComponent() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UMomentumComponent>();
}