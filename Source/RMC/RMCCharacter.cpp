// Copyright Epic Games, Inc. All Rights Reserved.

#include "RMCCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Rift/RiftComponent.h"
#include "Rift/RiftAnchor.h"
#include "Weapons/WeaponManagerComponent.h"
#include "Weapons/StyleComponent.h"
#include "Weapons/RangedWeaponBase.h"
#include "WallRunComponent.h"
#include "DoubleJumpComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/ComponentLocator.h"
#include "Core/GameEventSystem.h"
#include "Core/MomentumInterface.h"
#include "Core/RiftInterface.h"
#include "Core/StyleInterface.h"
#include "Core/WallRunInterface.h"
#include "Core/DoubleJumpInterface.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ARMCCharacter

ARMCCharacter::ARMCCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Create the component locator (must be created first for other components to register with it)
	ComponentLocator = CreateDefaultSubobject<UComponentLocator>(TEXT("ComponentLocator"));

	// Create the rift component
	RiftComponent = CreateDefaultSubobject<URiftComponent>(TEXT("RiftComponent"));

	// Create the weapon manager component
	WeaponManager = CreateDefaultSubobject<UWeaponManagerComponent>(TEXT("WeaponManager"));

	// Create the style component
	StyleComponent = CreateDefaultSubobject<UStyleComponent>(TEXT("StyleComponent"));

	// Create the momentum component
	MomentumComponent = CreateDefaultSubobject<UMomentumComponent>(TEXT("MomentumComponent"));

	// Create the wall run component
	WallRunComponent = CreateDefaultSubobject<UWallRunComponent>(TEXT("WallRunComponent"));

	// Create the double jump component
	DoubleJumpComponent = CreateDefaultSubobject<UDoubleJumpComponent>(TEXT("DoubleJumpComponent"));

	// Initialize event system reference
	EventSystem = nullptr;
}

void ARMCCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Get the event system
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		EventSystem = GameInstance->GetSubsystem<UGameEventSubsystem>();
		if (EventSystem)
		{
			// Register for events
			EventListenerHandles.Add(EventSystem->AddEventListenerWithObject(EGameEventType::MomentumChanged, this, FName("OnMomentumChanged")));
			EventListenerHandles.Add(EventSystem->AddEventListenerWithObject(EGameEventType::StylePointsGained, this, FName("OnStylePointsGained")));
			EventListenerHandles.Add(EventSystem->AddEventListenerWithObject(EGameEventType::RiftPerformed, this, FName("OnRiftPerformed")));
			EventListenerHandles.Add(EventSystem->AddEventListenerWithObject(EGameEventType::WallRunStarted, this, FName("OnWallRunStarted")));
			EventListenerHandles.Add(EventSystem->AddEventListenerWithObject(EGameEventType::WallRunEnded, this, FName("OnWallRunEnded")));
			EventListenerHandles.Add(EventSystem->AddEventListenerWithObject(EGameEventType::WeaponFired, this, FName("OnWeaponFired")));
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Warning, TEXT("%s: Failed to get GameEventSubsystem. Event-based communication will be disabled."), *GetNameSafe(this));
		}
	}

	// Add default weapons
	for (TSubclassOf<URangedWeaponBase> WeaponClass : DefaultWeapons)
	{
		if (WeaponClass && WeaponManager)
		{
			WeaponManager->AddWeaponToInventory(WeaponClass);
		}
	}
}

void ARMCCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

	// Call the base class
	Super::EndPlay(EndPlayReason);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARMCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARMCCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARMCCharacter::Look);

		// Phantom Dodge
		if (PhantomDodgeAction)
		{
			EnhancedInputComponent->BindAction(PhantomDodgeAction, ETriggerEvent::Started, this, &ARMCCharacter::PerformPhantomDodge);
		}

		// Rift Tether
		if (RiftTetherAction)
		{
			EnhancedInputComponent->BindAction(RiftTetherAction, ETriggerEvent::Started, this, &ARMCCharacter::PerformRiftTether);
		}

		// Weapon Fire
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ARMCCharacter::Fire);
		}

		// Weapon Alt Fire
		if (AltFireAction)
		{
			EnhancedInputComponent->BindAction(AltFireAction, ETriggerEvent::Started, this, &ARMCCharacter::AltFire);
			EnhancedInputComponent->BindAction(AltFireAction, ETriggerEvent::Completed, this, &ARMCCharacter::ReleaseCharge);
		}

		// Weapon Reload
		if (ReloadAction)
		{
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ARMCCharacter::Reload);
		}

		// Weapon Switch
		if (WeaponSwitchAction)
		{
			EnhancedInputComponent->BindAction(WeaponSwitchAction, ETriggerEvent::Started, this, &ARMCCharacter::SwitchWeapon);
		}

		// Wall Run
		if (WallRunAction)
		{
			EnhancedInputComponent->BindAction(WallRunAction, ETriggerEvent::Started, this, &ARMCCharacter::TryWallRun);
		}

		// Wall Jump
		if (WallJumpAction)
		{
			EnhancedInputComponent->BindAction(WallJumpAction, ETriggerEvent::Started, this, &ARMCCharacter::TryWallJump);
		}

		// Double Jump
		if (DoubleJumpAction)
		{
			EnhancedInputComponent->BindAction(DoubleJumpAction, ETriggerEvent::Started, this, &ARMCCharacter::TryDoubleJump);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARMCCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARMCCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARMCCharacter::PerformPhantomDodge(const FInputActionValue& Value)
{
	// For now, we'll use the direct component reference since we're having issues with the interface execution
	if (!RiftComponent)
	{
		return;
	}

	// Get movement direction
	FVector Direction = FVector::ZeroVector;

	// If we're providing input, use that direction
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}

	// If no direction, use actor forward
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}

	// Check if we're in the air
	bool bIsAerial = GetCharacterMovement() && GetCharacterMovement()->IsFalling();

	// Check if we have enough momentum for aerial dodge if applicable
	if (bIsAerial && MomentumComponent)
	{
		// Aerial dodges might require a minimum momentum tier
		int32 CurrentTier = MomentumComponent->GetMomentumTier();
		if (CurrentTier < 1) // Require at least tier 1 for aerial dodges
		{
			// Not enough momentum for aerial dodge
			return;
		}
	}

	// Perform the phantom dodge
	if (RiftComponent->PerformPhantomDodge(Direction, bIsAerial))
	{
		// Get style value for phantom dodge
		float StyleValue = 0.0f;
		if (StyleComponent)
		{
			StyleValue = StyleComponent->GetMoveStyleValue(FName("PhantomDodge"));
		}
		
		// Add style points for successful dodge
		AddStylePoints(StyleValue, FName("PhantomDodge"));
		
		// Add additional momentum directly for successful dodge
		if (MomentumComponent)
		{
			// Add more momentum for aerial dodges to encourage stylish play
			float MomentumBonus = bIsAerial ? 15.0f : 10.0f;
			MomentumComponent->AddMomentum(MomentumBonus, FName("PhantomDodge"));
		}
		
		// Broadcast rift performed event
		BroadcastGameEvent(EGameEventType::RiftPerformed, 0.0f, 0, FName("PhantomDodge"));
	}
}

void ARMCCharacter::PerformRiftTether(const FInputActionValue& Value)
{
	// For now, we'll use the direct component reference since we're having issues with the interface execution
	if (!RiftComponent)
	{
		return;
	}

	// Find the best rift anchor in range
	ARiftAnchor* BestAnchor = FindBestRiftAnchor();

	// If we found an anchor, tether to it
	if (BestAnchor)
	{
		// Check if we have enough momentum for this anchor
		if (BestAnchor->RequiredMomentum > 0 && MomentumComponent)
		{
			// Make sure we meet the momentum requirement
			if (MomentumComponent->CurrentMomentum < BestAnchor->RequiredMomentum)
			{
				// Not enough momentum for this anchor
				return;
			}
		}

		if (RiftComponent->InitiateRiftTetherToAnchor(BestAnchor))
		{
			// Get style value for rift tether
			float StyleValue = 0.0f;
			if (StyleComponent)
			{
				StyleValue = StyleComponent->GetMoveStyleValue(FName("RiftTether"));
			}
			
			// Add style points for successful tether
			AddStylePoints(StyleValue, FName("RiftTether"));
			
			// Add additional momentum directly for successful tether
			if (MomentumComponent)
			{
				MomentumComponent->AddMomentum(12.0f, FName("RiftTether"));
			}
			
			// Broadcast rift performed event
			BroadcastGameEvent(EGameEventType::RiftPerformed, 0.0f, 0, FName("RiftTether"), BestAnchor);
		}
	}
	// Otherwise, try to chain if we're already tethering
	else if (RiftComponent->GetRiftState() == ERiftState::Tethering)
	{
		// Check if we have enough momentum for chaining
		if (MomentumComponent)
		{
			int32 CurrentTier = MomentumComponent->GetMomentumTier();
			int32 RequiredTier = 1; // Require at least tier 1 for basic chaining
			
			// For multiple chains, require higher tiers
			if (RiftComponent->CurrentChainCount > 0)
			{
				RequiredTier = 2; // Tier 2 for second chain
			}
			if (RiftComponent->CurrentChainCount > 1)
			{
				RequiredTier = 3; // Tier 3 for third chain and beyond
			}
			
			if (CurrentTier < RequiredTier)
			{
				// Not enough momentum for chaining
				return;
			}
		}

		if (RiftComponent->ChainRiftTether())
		{
			// Get style value for chain rift
			float StyleValue = 0.0f;
			if (StyleComponent)
			{
				StyleValue = StyleComponent->GetMoveStyleValue(FName("ChainRift"));
			}
			
			// Add style points for successful chain
			AddStylePoints(StyleValue, FName("ChainRift"));
			
			// Add additional momentum directly for successful chain
			// Give more momentum for chains to encourage combo play
			if (MomentumComponent)
			{
				MomentumComponent->AddMomentum(18.0f, FName("ChainRift"));
			}
			
			// Broadcast rift performed event
			BroadcastGameEvent(EGameEventType::RiftPerformed, 0.0f, 0, FName("ChainRift"));
		}
	}
}

void ARMCCharacter::Fire(const FInputActionValue& Value)
{
	// For now, we'll keep using the direct WeaponManager reference
	// since we haven't created a WeaponManagerInterface yet
	if (WeaponManager)
	{
		// Apply momentum-based damage multiplier if available
		if (MomentumComponent)
		{
			// Get the current weapon
			URangedWeaponBase* CurrentWeapon = WeaponManager->CurrentWeapon;
			if (CurrentWeapon)
			{
				// Store original damage
				float OriginalDamage = CurrentWeapon->BaseDamage;
				
				// Apply momentum-based damage multiplier
				float DamageMultiplier = MomentumComponent->GetDamageMultiplier();
				CurrentWeapon->BaseDamage *= DamageMultiplier;
				
				// Fire the weapon
				WeaponManager->FireCurrentWeapon();
				
				// Restore original damage
				CurrentWeapon->BaseDamage = OriginalDamage;
				
				// Small momentum gain just for firing (encourages active play)
				MomentumComponent->AddMomentum(2.0f, FName("WeaponFire"));
				
				// Broadcast weapon fired event
				BroadcastGameEvent(EGameEventType::WeaponFired, 0.0f, 0, FName("WeaponFire"));
				return;
			}
		}
		
		// If no momentum component or current weapon, just fire normally
		WeaponManager->FireCurrentWeapon();
		
		// Broadcast weapon fired event
		BroadcastGameEvent(EGameEventType::WeaponFired, 0.0f, 0, FName("WeaponFire"));
		
		// Style points will be added when the projectile hits a target
		// This is handled in the projectile's OnHit function
	}
}

void ARMCCharacter::AltFire(const FInputActionValue& Value)
{
	if (WeaponManager)
	{
		WeaponManager->StartChargeCurrentWeapon();
		
		// Small momentum gain for starting a charge (encourages charged shots)
		if (MomentumComponent)
		{
			MomentumComponent->AddMomentum(1.0f, FName("StartCharge"));
		}
	}
}

void ARMCCharacter::ReleaseCharge(const FInputActionValue& Value)
{
	if (WeaponManager)
	{
		// Apply momentum-based damage multiplier if available
		if (MomentumComponent)
		{
			// Get the current weapon
			URangedWeaponBase* CurrentWeapon = WeaponManager->CurrentWeapon;
			if (CurrentWeapon)
			{
				// Store original damage
				float OriginalDamage = CurrentWeapon->BaseDamage;
				
				// Apply momentum-based damage multiplier with bonus for charged shots
				float DamageMultiplier = MomentumComponent->GetDamageMultiplier() * 1.2f;
				CurrentWeapon->BaseDamage *= DamageMultiplier;
				
				// Release the charge
				WeaponManager->ReleaseChargeCurrentWeapon();
				
				// Restore original damage
				CurrentWeapon->BaseDamage = OriginalDamage;
				
				// Momentum gain for charged shots (more than regular shots)
				MomentumComponent->AddMomentum(5.0f, FName("ChargedShot"));
				
				// Broadcast weapon fired event
				BroadcastGameEvent(EGameEventType::WeaponFired, 0.0f, 0, FName("ChargedShot"));
				return;
			}
		}
		
		// If no momentum component or current weapon, just release normally
		WeaponManager->ReleaseChargeCurrentWeapon();
		
		// Broadcast weapon fired event
		BroadcastGameEvent(EGameEventType::WeaponFired, 0.0f, 0, FName("ChargedShot"));
		
		// Style points will be added when the projectile hits a target
		// This is handled in the projectile's OnHit function
	}
}

void ARMCCharacter::Reload(const FInputActionValue& Value)
{
	if (WeaponManager)
	{
		WeaponManager->StartReloadCurrentWeapon();
		
		// Broadcast weapon reloaded event
		BroadcastGameEvent(EGameEventType::WeaponReloaded, 0.0f, 0, FName("WeaponReload"));
	}
}

void ARMCCharacter::SwitchWeapon(const FInputActionValue& Value)
{
	if (WeaponManager)
	{
		// Get the input value (can be used to determine direction of switch)
		float SwitchDirection = Value.Get<float>();
		
		if (SwitchDirection > 0.0f)
		{
			WeaponManager->NextWeapon();
		}
		else
		{
			WeaponManager->PreviousWeapon();
		}
		
		// We could broadcast a weapon switched event here, but we'll need to add it to the event types first
		// For now, we'll rely on the WeaponManager's OnWeaponSwitched delegate
	}
}

// Note: We're temporarily commenting out these interface methods due to linker errors
// We'll use direct component references for now
/*
// Helper methods for getting interfaces
IMomentumInterface* ARMCCharacter::GetMomentumInterface() const
{
	// First try to get from component locator
	if (ComponentLocator)
	{
		UActorComponent* Component = ComponentLocator->GetComponentByClass(UMomentumComponent::StaticClass());
		if (Component)
		{
			return Cast<IMomentumInterface>(Component);
		}
	}
	
	// Fallback to direct reference
	if (MomentumComponent)
	{
		return Cast<IMomentumInterface>(MomentumComponent);
	}
	
	return nullptr;
}

IRiftInterface* ARMCCharacter::GetRiftInterface() const
{
	// First try to get from component locator
	if (ComponentLocator)
	{
		UActorComponent* Component = ComponentLocator->GetComponentByClass(URiftComponent::StaticClass());
		if (Component)
		{
			return Cast<IRiftInterface>(Component);
		}
	}
	
	// Fallback to direct reference
	if (RiftComponent)
	{
		return Cast<IRiftInterface>(RiftComponent);
	}
	
	return nullptr;
}

IStyleInterface* ARMCCharacter::GetStyleInterface() const
{
	// First try to get from component locator
	if (ComponentLocator)
	{
		UActorComponent* Component = ComponentLocator->GetComponentByClass(UStyleComponent::StaticClass());
		if (Component)
		{
			return Cast<IStyleInterface>(Component);
		}
	}
	
	// Fallback to direct reference
	if (StyleComponent)
	{
		return Cast<IStyleInterface>(StyleComponent);
	}
	
	return nullptr;
}

IWallRunInterface* ARMCCharacter::GetWallRunInterface() const
{
	// First try to get from component locator
	if (ComponentLocator)
	{
		UActorComponent* Component = ComponentLocator->GetComponentByClass(UWallRunComponent::StaticClass());
		if (Component)
		{
			return Cast<IWallRunInterface>(Component);
		}
	}
	
	// Fallback to direct reference
	if (WallRunComponent)
	{
		return Cast<IWallRunInterface>(WallRunComponent);
	}
	
	return nullptr;
}
*/

// Helper method to broadcast events
void ARMCCharacter::BroadcastGameEvent(EGameEventType EventType, float FloatValue, int32 IntValue, FName NameValue, AActor* Target)
{
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EventType;
		EventData.Instigator = this;
		EventData.Target = Target;
		EventData.FloatValue = FloatValue;
		EventData.IntValue = IntValue;
		EventData.NameValue = NameValue;
		EventSystem->BroadcastEvent(EventData);
	}
}

// Event handlers
void ARMCCharacter::OnMomentumChanged(const FGameEventData& EventData)
{
	// This event is broadcast when momentum changes
	// We can use this to update character visuals, sounds, etc. based on momentum
	
	// For now, we'll just log the event
	UE_LOG(LogTemplateCharacter, Verbose, TEXT("Momentum changed: %f"), EventData.FloatValue);
}

void ARMCCharacter::OnStylePointsGained(const FGameEventData& EventData)
{
	// This event is broadcast when style points are gained
	// We can use this to update character visuals, sounds, etc. based on style
	
	// For now, we'll just log the event
	UE_LOG(LogTemplateCharacter, Verbose, TEXT("Style points gained: %f from %s"), 
		EventData.FloatValue, *EventData.NameValue.ToString());
}

void ARMCCharacter::OnRiftPerformed(const FGameEventData& EventData)
{
	// This event is broadcast when a rift action is performed
	// We can use this to update character visuals, sounds, etc. based on rift actions
	
	// For now, we'll just log the event
	UE_LOG(LogTemplateCharacter, Verbose, TEXT("Rift performed: %s"), *EventData.NameValue.ToString());
}

void ARMCCharacter::OnWallRunStarted(const FGameEventData& EventData)
{
	// This event is broadcast when wall running starts
	// We can use this to update character visuals, sounds, etc. based on wall running
	
	// For now, we'll just log the event
	UE_LOG(LogTemplateCharacter, Verbose, TEXT("Wall run started"));
}

void ARMCCharacter::OnWallRunEnded(const FGameEventData& EventData)
{
	// This event is broadcast when wall running ends
	// We can use this to update character visuals, sounds, etc. based on wall running
	
	// For now, we'll just log the event
	UE_LOG(LogTemplateCharacter, Verbose, TEXT("Wall run ended"));
}

void ARMCCharacter::OnWeaponFired(const FGameEventData& EventData)
{
	// This event is broadcast when a weapon is fired
	// We can use this to update character visuals, sounds, etc. based on weapon firing
	
	// For now, we'll just log the event
	UE_LOG(LogTemplateCharacter, Verbose, TEXT("Weapon fired: %s"), *EventData.NameValue.ToString());
}

ARiftAnchor* ARMCCharacter::FindBestRiftAnchor() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	// Get all rift anchors in the world
	TArray<AActor*> FoundAnchors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARiftAnchor::StaticClass(), FoundAnchors);

	if (FoundAnchors.Num() == 0)
	{
		return nullptr;
	}

	// Get player view point
	FVector CameraLocation;
	FRotator CameraRotation;
	if (Controller && Controller->IsPlayerController())
	{
		Cast<APlayerController>(Controller)->GetPlayerViewPoint(CameraLocation, CameraRotation);
	}
	else
	{
		CameraLocation = GetActorLocation();
		CameraRotation = GetActorRotation();
	}

	// Get forward vector
	FVector ForwardVector = CameraRotation.Vector();

	// Find the anchor that best matches our view direction and is within range
	ARiftAnchor* BestAnchor = nullptr;
	float BestScore = -1.0f;
	float MaxDistance = 1000.0f; // Default max distance

	// If we have a rift component, use its max distance
	if (RiftComponent)
	{
		MaxDistance = RiftComponent->GetRiftCapabilities().MaxRiftDistance;
	}

	// Get current momentum for anchor compatibility check
	float CurrentMomentum = 0.0f;
	if (MomentumComponent)
	{
		CurrentMomentum = MomentumComponent->CurrentMomentum;
	}

	for (AActor* Actor : FoundAnchors)
	{
		ARiftAnchor* Anchor = Cast<ARiftAnchor>(Actor);
		if (!Anchor || !Anchor->CanUseWithMomentum(CurrentMomentum))
		{
			continue;
		}

		// Calculate distance and direction to anchor
		FVector ToAnchor = Anchor->GetActorLocation() - CameraLocation;
		float Distance = ToAnchor.Size();

		// Skip if too far
		if (Distance > MaxDistance)
		{
			continue;
		}

		// Normalize
		ToAnchor.Normalize();

		// Calculate dot product to see how closely it aligns with our view
		float DotProduct = FVector::DotProduct(ForwardVector, ToAnchor);

		// Skip if behind us
		if (DotProduct < 0.5f) // Roughly 60 degree cone
		{
			continue;
		}

		// Score based on alignment and distance (prefer closer anchors)
		float Score = DotProduct * (1.0f - (Distance / MaxDistance));

		if (Score > BestScore)
		{
			BestScore = Score;
			BestAnchor = Anchor;
		}
	}

	return BestAnchor;
}

void ARMCCharacter::AddStylePoints(float Points, FName MoveName)
{
	// Broadcast style points gained event through the event system
	if (EventSystem)
	{
		FGameEventData EventData;
		EventData.EventType = EGameEventType::StylePointsGained;
		EventData.Instigator = this;
		EventData.FloatValue = Points;
		EventData.NameValue = MoveName;
		EventSystem->BroadcastEvent(EventData);
	}
	
	// For backward compatibility, also call the direct methods
	// These will eventually be removed once all components are updated to use the event system
	if (StyleComponent)
	{
		StyleComponent->AddStylePoints(Points, MoveName);
	}

	// Also add momentum when gaining style points
	if (MomentumComponent)
	{
		// Convert style points to momentum (at a reduced rate)
		float MomentumGain = Points * 0.2f;
		MomentumComponent->AddMomentum(MomentumGain, MoveName);
	}
}

void ARMCCharacter::TryWallRun(const FInputActionValue& Value)
{
	// For now, we'll use the direct component reference since we're having issues with the interface execution
	if (!WallRunComponent)
	{
		return;
	}

	// Try to start wall running
	if (WallRunComponent->TryStartWallRun())
	{
		// Get style value for wall run
		float StyleValue = 0.0f;
		if (StyleComponent)
		{
			StyleValue = StyleComponent->GetMoveStyleValue(FName("WallRun"));
		}
		
		// Add style points for successful wall run
		AddStylePoints(StyleValue, FName("WallRun"));

		// Broadcast wall run started event
		BroadcastGameEvent(EGameEventType::WallRunStarted, 0.0f, 0, FName("WallRun"));
		
		// Additional momentum gain is handled in the WallRunComponent
	}
}

void ARMCCharacter::TryWallJump(const FInputActionValue& Value)
{
	// For now, we'll use the direct component reference since we're having issues with the interface execution
	if (!WallRunComponent)
	{
		return;
	}

	// Try to perform wall jump
	if (WallRunComponent->PerformWallJump())
	{
		// Get style value for wall jump
		float StyleValue = 0.0f;
		if (StyleComponent)
		{
			StyleValue = StyleComponent->GetMoveStyleValue(FName("WallJump"));
		}
		
		// Add style points for successful wall jump
		AddStylePoints(StyleValue, FName("WallJump"));
		
		// Broadcast wall jump event
		BroadcastGameEvent(EGameEventType::WallRunEnded, 0.0f, 0, FName("WallJump"));

		// Additional momentum gain is handled in the WallRunComponent
	}
}

void ARMCCharacter::TryDoubleJump(const FInputActionValue& Value)
{
	// For now, we'll use the direct component reference since we're having issues with the interface execution
	if (!DoubleJumpComponent)
	{
		return;
	}

	// Try to perform double jump
	if (DoubleJumpComponent->PerformDoubleJump())
	{
		// Get style value for double jump
		float StyleValue = 0.0f;
		if (StyleComponent)
		{
			StyleValue = StyleComponent->GetMoveStyleValue(FName("DoubleJump"));
			if (StyleValue <= 0.0f)
			{
				// Default style value if not defined
				StyleValue = 100.0f;
			}
		}
		
		// Add style points for successful double jump
		AddStylePoints(StyleValue, FName("DoubleJump"));
		
		// Broadcast double jump event (using wall run ended event type for now)
		BroadcastGameEvent(EGameEventType::WallRunEnded, 0.0f, 0, FName("DoubleJump"));

		// Additional momentum gain is handled in the DoubleJumpComponent
	}
}

float ARMCCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Call parent implementation
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Only process damage if it's greater than 0
	if (ActualDamage > 0.0f)
	{
		// Broadcast damage event through the event system
		if (EventSystem)
		{
			FGameEventData EventData;
			EventData.EventType = EGameEventType::PlayerDamaged;
			EventData.Instigator = DamageCauser;
			EventData.Target = this;
			EventData.FloatValue = ActualDamage;
			EventSystem->BroadcastEvent(EventData);
		}
		
		// For backward compatibility, also call the direct methods
		// These will eventually be removed once all components are updated to use the event system
		if (StyleComponent)
		{
			StyleComponent->TakeDamage(ActualDamage);
		}

		if (MomentumComponent)
		{
			MomentumComponent->OnTakeDamage(ActualDamage);
		}
	}

	return ActualDamage;
}