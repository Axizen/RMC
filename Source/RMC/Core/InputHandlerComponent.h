// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameEventSystem.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "InputHandlerComponent.generated.h"

class UInputAction;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

/**
 * Enum defining different input contexts for the game.
 * Each context represents a different state with its own input mappings.
 */
UENUM(BlueprintType)
enum class EInputContextType : uint8
{
	Normal UMETA(DisplayName = "Normal Gameplay"),
	Combat UMETA(DisplayName = "Combat"),
	Rifting UMETA(DisplayName = "Rifting"),
	WallRunning UMETA(DisplayName = "Wall Running"),
	Vehicle UMETA(DisplayName = "Vehicle"),
	Menu UMETA(DisplayName = "Menu"),
	Dialog UMETA(DisplayName = "Dialog"),
	Disabled UMETA(DisplayName = "Disabled")
};

/**
 * Struct containing data about an input action.
 * This is used to map input actions to game events.
 */
USTRUCT(BlueprintType)
struct FInputActionData
{
	GENERATED_BODY()

	// The input action asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* InputAction = nullptr;

	// The event type to broadcast when this action is triggered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EGameEventType EventType = EGameEventType::MomentumChanged;

	// The trigger event to listen for (Started, Triggered, Completed, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	ETriggerEvent TriggerEvent = ETriggerEvent::Started;

	// Whether this action should be buffered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bShouldBuffer = false;

	// The buffer time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float BufferTime = 0.2f;

	// The contexts in which this action is valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<EInputContextType> ValidContexts;

	// Default constructor
	FInputActionData()
	{
		ValidContexts.Add(EInputContextType::Normal);
	}
};

/**
 * Struct containing data about a buffered input.
 * This is used to store inputs for later execution.
 */
USTRUCT()
struct FBufferedInput
{
	GENERATED_BODY()

	// The event data to broadcast
	FGameEventData EventData;

	// The time when the input was buffered
	float BufferTime = 0.0f;

	// The time when the buffer expires
	float ExpirationTime = 0.0f;

	// Whether the input has been consumed
	bool bConsumed = false;
};

/**
 * InputHandlerComponent
 * 
 * This component handles input in a decoupled way, broadcasting events
 * instead of directly calling functions on other components.
 * It supports input contexts, buffering, and dynamic rebinding.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UInputHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInputHandlerComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Called when the component is registered
	virtual void OnRegister() override;

	// Called when the component is unregistered
	virtual void OnUnregister() override;

	// Called when the component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Input mapping contexts for different input contexts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TMap<EInputContextType, UInputMappingContext*> InputContexts;

	// Priority for each input context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TMap<EInputContextType, int32> ContextPriorities;

	// Input action mappings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<FInputActionData> InputActions;

	// Current active input context
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	EInputContextType CurrentContext;

	// Previous input context (for returning from temporary contexts)
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	EInputContextType PreviousContext;

	// Whether input buffering is enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bEnableInputBuffering;

	// Maximum buffer time for buffered inputs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	float MaxBufferTime;

	// Sets the current input context
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputContext(EInputContextType NewContext, bool bRememberPrevious = true);

	// Returns to the previous input context
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ReturnToPreviousContext();

	// Temporarily sets an input context and automatically returns to the previous one after a delay
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetTemporaryContext(EInputContextType NewContext, float Duration);

	// Disables all input
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DisableInput();

	// Enables input (returns to previous context)
	UFUNCTION(BlueprintCallable, Category = "Input")
	void EnableInput();

	// Rebinds an input action to a new event type
	UFUNCTION(BlueprintCallable, Category = "Input")
	void RebindInputAction(UInputAction* InputAction, EGameEventType NewEventType);

	// Checks if an input action is valid in the current context
	UFUNCTION(BlueprintPure, Category = "Input")
	bool IsActionValidInContext(const FInputActionData& ActionData) const;

	// Consumes a buffered input if available
	UFUNCTION(BlueprintCallable, Category = "Input")
	bool ConsumeBufferedInput(EGameEventType EventType);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Setup input component
	void SetupInputComponent();

	// Generic input handler
	void HandleInput(const FInputActionData& ActionData, const FInputActionValue& Value);

	// Buffer an input for later execution
	void BufferInput(const FGameEventData& EventData, float BufferDuration);

	// Process buffered inputs
	void ProcessBufferedInputs();

	// Clear expired buffered inputs
	void ClearExpiredBufferedInputs();

	// Apply the current input context
	void ApplyCurrentContext();

	// Event system reference
	UPROPERTY()
	UGameEventSubsystem* EventSystem;

	// Enhanced input component reference
	UPROPERTY()
	UEnhancedInputComponent* EnhancedInputComponent;

	// Enhanced input subsystem reference
	UPROPERTY()
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem;

	// Buffered inputs
	TArray<FBufferedInput> BufferedInputs;

	// Timer handle for temporary context
	FTimerHandle TemporaryContextTimerHandle;

	// Callback for when temporary context expires
	UFUNCTION()
	void OnTemporaryContextExpired();

	// Event listener handles
	TArray<FDelegateHandle> EventListenerHandles;

	// Event handlers
	UFUNCTION()
	void OnGameStateChanged(const FGameEventData& EventData);

	// Helper function to get player controller
	APlayerController* GetOwnerPlayerController() const;
};