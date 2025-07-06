// Fill out your copyright notice in the Description page of Project Settings.

#include "InputHandlerComponent.h"
#include "ComponentLocator.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UInputHandlerComponent::UInputHandlerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	CurrentContext = EInputContextType::Normal;
	PreviousContext = EInputContextType::Normal;
	bEnableInputBuffering = true;
	MaxBufferTime = 0.5f;

	// Set default context priorities
	ContextPriorities.Add(EInputContextType::Normal, 0);
	ContextPriorities.Add(EInputContextType::Combat, 1);
	ContextPriorities.Add(EInputContextType::Rifting, 2);
	ContextPriorities.Add(EInputContextType::WallRunning, 2);
	ContextPriorities.Add(EInputContextType::Vehicle, 1);
	ContextPriorities.Add(EInputContextType::Menu, 10);
	ContextPriorities.Add(EInputContextType::Dialog, 11);
	ContextPriorities.Add(EInputContextType::Disabled, 100);
}

// Called when the component is registered
void UInputHandlerComponent::OnRegister()
{
	Super::OnRegister();

	// Register with the component locator
	UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		Locator->RegisterComponent(this, TEXT("InputHandlerComponent"));
	}
}

// Called when the component is unregistered
void UInputHandlerComponent::OnUnregister()
{
	// Unregister from the component locator
	UComponentLocator* Locator = GetOwner()->FindComponentByClass<UComponentLocator>();
	if (Locator)
	{
		Locator->UnregisterComponent(TEXT("InputHandlerComponent"));
	}

	Super::OnUnregister();
}

// Called when the game starts
void UInputHandlerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get the event system
	EventSystem = GetWorld()->GetGameInstance()->GetSubsystem<UGameEventSubsystem>();
	if (EventSystem)
	{
		// Register for events
		EventListenerHandles.Add(EventSystem->AddEventListener(this, EGameEventType::GameStateChanged, 
			FName("OnGameStateChanged")));
	}

	// Setup input component
	SetupInputComponent();

	// Apply the initial context
	ApplyCurrentContext();
}

// Called when the component is destroyed
void UInputHandlerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up event listeners
	if (EventSystem)
	{
		for (const FDelegateHandle& Handle : EventListenerHandles)
		{
			EventSystem->RemoveEventListener(Handle);
		}
		EventListenerHandles.Empty();
	}

	// Clear timer
	GetWorld()->GetTimerManager().ClearTimer(TemporaryContextTimerHandle);

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UInputHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Process buffered inputs
	if (bEnableInputBuffering)
	{
		ProcessBufferedInputs();
		ClearExpiredBufferedInputs();
	}
}

void UInputHandlerComponent::SetupInputComponent()
{
	// Get the player controller
	APlayerController* PC = GetOwnerPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to get player controller"), *GetNameSafe(this));
		return;
	}

	// Get the enhanced input component
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to get enhanced input component"), *GetNameSafe(this));
		return;
	}

	// Get the enhanced input subsystem
	InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!InputSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Failed to get enhanced input subsystem"), *GetNameSafe(this));
		return;
	}

	// Bind all input actions
	for (const FInputActionData& ActionData : InputActions)
	{
		if (ActionData.InputAction)
		{
			// Create a lambda that captures the action data
			FInputActionHandlerDynamicSignature Handler;
			Handler.BindUObject(this, &UInputHandlerComponent::HandleInput, ActionData);

			// Bind the action
			EnhancedInputComponent->BindAction(ActionData.InputAction, ActionData.TriggerEvent, Handler);
		}
	}
}

void UInputHandlerComponent::HandleInput(const FInputActionData& ActionData, const FInputActionValue& Value)
{
	// Check if the action is valid in the current context
	if (!IsActionValidInContext(ActionData))
	{
		// If input buffering is enabled and the action should be buffered, buffer it
		if (bEnableInputBuffering && ActionData.bShouldBuffer)
		{
			// Create event data
			FGameEventData EventData;
			EventData.EventType = ActionData.EventType;
			EventData.Instigator = GetOwner();

			// Store input value based on type
			if (Value.GetValueType() == EInputActionValueType::Boolean)
			{
				EventData.BoolValue = Value.Get<bool>();
			}
			else if (Value.GetValueType() == EInputActionValueType::Axis1D)
			{
				EventData.FloatValue = Value.Get<float>();
			}
			else if (Value.GetValueType() == EInputActionValueType::Axis2D)
			{
				EventData.VectorValue = FVector(Value.Get<FVector2D>().X, Value.Get<FVector2D>().Y, 0.0f);
			}
			else if (Value.GetValueType() == EInputActionValueType::Axis3D)
			{
				EventData.VectorValue = Value.Get<FVector>();
			}

			// Buffer the input
			BufferInput(EventData, ActionData.BufferTime);
		}
		return;
	}

	// Create event data
	FGameEventData EventData;
	EventData.EventType = ActionData.EventType;
	EventData.Instigator = GetOwner();

	// Store input value based on type
	if (Value.GetValueType() == EInputActionValueType::Boolean)
	{
		EventData.BoolValue = Value.Get<bool>();
	}
	else if (Value.GetValueType() == EInputActionValueType::Axis1D)
	{
		EventData.FloatValue = Value.Get<float>();
	}
	else if (Value.GetValueType() == EInputActionValueType::Axis2D)
	{
		EventData.VectorValue = FVector(Value.Get<FVector2D>().X, Value.Get<FVector2D>().Y, 0.0f);
	}
	else if (Value.GetValueType() == EInputActionValueType::Axis3D)
	{
		EventData.VectorValue = Value.Get<FVector>();
	}

	// Broadcast the event
	if (EventSystem)
	{
		EventSystem->BroadcastEvent(EventData);
	}
}

void UInputHandlerComponent::BufferInput(const FGameEventData& EventData, float BufferDuration)
{
	// Create a new buffered input
	FBufferedInput BufferedInput;
	BufferedInput.EventData = EventData;
	BufferedInput.BufferTime = GetWorld()->GetTimeSeconds();
	BufferedInput.ExpirationTime = BufferedInput.BufferTime + BufferDuration;
	BufferedInput.bConsumed = false;

	// Add to the buffer
	BufferedInputs.Add(BufferedInput);
}

void UInputHandlerComponent::ProcessBufferedInputs()
{
	// Get current time
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Process each buffered input
	for (FBufferedInput& BufferedInput : BufferedInputs)
	{
		// Skip if already consumed or expired
		if (BufferedInput.bConsumed || CurrentTime > BufferedInput.ExpirationTime)
		{
			continue;
		}

		// Check if the action is valid in the current context
		bool bIsValid = false;
		for (const FInputActionData& ActionData : InputActions)
		{
			if (ActionData.EventType == BufferedInput.EventData.EventType && IsActionValidInContext(ActionData))
			{
				bIsValid = true;
				break;
			}
		}

		// If valid, broadcast the event
		if (bIsValid && EventSystem)
		{
			EventSystem->BroadcastEvent(BufferedInput.EventData);
			BufferedInput.bConsumed = true;
		}
	}
}

void UInputHandlerComponent::ClearExpiredBufferedInputs()
{
	// Get current time
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Remove expired or consumed inputs
	BufferedInputs.RemoveAll([CurrentTime](const FBufferedInput& Input) {
		return Input.bConsumed || CurrentTime > Input.ExpirationTime;
	});
}

void UInputHandlerComponent::SetInputContext(EInputContextType NewContext, bool bRememberPrevious)
{
	// Store previous context if requested
	if (bRememberPrevious)
	{
		PreviousContext = CurrentContext;
	}

	// Set new context
	CurrentContext = NewContext;

	// Apply the context
	ApplyCurrentContext();
}

void UInputHandlerComponent::ReturnToPreviousContext()
{
	// Swap current and previous contexts
	EInputContextType TempContext = CurrentContext;
	CurrentContext = PreviousContext;
	PreviousContext = TempContext;

	// Apply the context
	ApplyCurrentContext();
}

void UInputHandlerComponent::SetTemporaryContext(EInputContextType NewContext, float Duration)
{
	// Store previous context
	PreviousContext = CurrentContext;

	// Set new context
	CurrentContext = NewContext;

	// Apply the context
	ApplyCurrentContext();

	// Set timer to return to previous context
	GetWorld()->GetTimerManager().SetTimer(
		TemporaryContextTimerHandle,
		this,
		&UInputHandlerComponent::OnTemporaryContextExpired,
		Duration,
		false);
}

void UInputHandlerComponent::OnTemporaryContextExpired()
{
	// Return to previous context
	ReturnToPreviousContext();
}

void UInputHandlerComponent::DisableInput()
{
	// Store previous context
	PreviousContext = CurrentContext;

	// Set disabled context
	CurrentContext = EInputContextType::Disabled;

	// Apply the context
	ApplyCurrentContext();
}

void UInputHandlerComponent::EnableInput()
{
	// Return to previous context
	ReturnToPreviousContext();
}

void UInputHandlerComponent::RebindInputAction(UInputAction* InputAction, EGameEventType NewEventType)
{
	// Find the action data
	for (FInputActionData& ActionData : InputActions)
	{
		if (ActionData.InputAction == InputAction)
		{
			// Update the event type
			ActionData.EventType = NewEventType;
			break;
		}
	}
}

bool UInputHandlerComponent::IsActionValidInContext(const FInputActionData& ActionData) const
{
	// Check if the action is valid in the current context
	return ActionData.ValidContexts.Contains(CurrentContext);
}

bool UInputHandlerComponent::ConsumeBufferedInput(EGameEventType EventType)
{
	// Find a buffered input with the specified event type
	for (FBufferedInput& BufferedInput : BufferedInputs)
	{
		if (!BufferedInput.bConsumed && BufferedInput.EventData.EventType == EventType)
		{
			// Mark as consumed
			BufferedInput.bConsumed = true;
			return true;
		}
	}

	return false;
}

void UInputHandlerComponent::ApplyCurrentContext()
{
	// Skip if input subsystem is not valid
	if (!InputSubsystem)
	{
		return;
	}

	// Clear all mapping contexts
	InputSubsystem->ClearAllMappingContexts();

	// Apply the current context
	UInputMappingContext* CurrentMappingContext = InputContexts.FindRef(CurrentContext);
	if (CurrentMappingContext)
	{
		int32 Priority = ContextPriorities.Contains(CurrentContext) ? ContextPriorities[CurrentContext] : 0;
		InputSubsystem->AddMappingContext(CurrentMappingContext, Priority);
	}

	// Log the context change
	UE_LOG(LogTemp, Verbose, TEXT("%s: Input context changed to %s"), 
		*GetNameSafe(this), *UEnum::GetValueAsString(CurrentContext));
}

void UInputHandlerComponent::OnGameStateChanged(const FGameEventData& EventData)
{
	// Handle game state changes that might affect input context
	// This is just an example - you would implement this based on your game's needs
	// For example, if the player enters combat, you might switch to the combat context

	// Example:
	// if (EventData.NameValue == FName("EnterCombat"))
	// {
	//     SetInputContext(EInputContextType::Combat);
	// }
	// else if (EventData.NameValue == FName("ExitCombat"))
	// {
	//     SetInputContext(EInputContextType::Normal);
	// }
}

APlayerController* UInputHandlerComponent::GetOwnerPlayerController() const
{
	// Try to get the player controller from the owner
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		return Cast<APlayerController>(OwnerPawn->GetController());
	}

	// If the owner is not a pawn, try to get the first player controller
	return UGameplayStatics::GetPlayerController(GetWorld(), 0);
}