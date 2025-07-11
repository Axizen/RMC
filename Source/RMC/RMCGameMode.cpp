// Fill out your copyright notice in the Description page of Project Settings.

#include "RMCGameMode.h"
#include "RMCCharacter.h"
#include "RMCPlayerController.h"
#include "Components/Movement/RMCMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ARMCGameMode::ARMCGameMode()
{
	// Set default classes
	DefaultPawnClass = ARMCCharacter::StaticClass();
	PlayerControllerClass = ARMCPlayerController::StaticClass();

	// Set default values
	bDebugModeEnabled = false;
	StartingMomentum = 0.0f;
	bGameStarted = false;
}

void ARMCGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Call OnGameStart event
	bGameStarted = true;
	OnGameStart();

	// Apply starting momentum to existing characters
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARMCCharacter::StaticClass(), FoundCharacters);

	for (AActor* Actor : FoundCharacters)
	{
		ARMCCharacter* Character = Cast<ARMCCharacter>(Actor);
		if (Character)
		{
			ApplyStartingMomentumToCharacter(Character);
		}
	}
}

void ARMCGameMode::OnGameStart_Implementation()
{
	// Default implementation - can be overridden in Blueprints
	UE_LOG(LogTemp, Display, TEXT("RMC Game Started"));

	// Enable debug mode if needed
	if (bDebugModeEnabled)
	{
		UE_LOG(LogTemp, Display, TEXT("Debug Mode Enabled"));
		
		// Find all player controllers and enable debug info
		TArray<AActor*> FoundControllers;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARMCPlayerController::StaticClass(), FoundControllers);

		for (AActor* Actor : FoundControllers)
		{
			ARMCPlayerController* Controller = Cast<ARMCPlayerController>(Actor);
			if (Controller)
			{
				Controller->bShowDebugInfo = true;
			}
		}
	}
}

void ARMCGameMode::OnPlayerReachedMaxMomentum_Implementation(ARMCCharacter* Character)
{
	// Default implementation - can be overridden in Blueprints
	if (Character && bDebugModeEnabled)
	{
		UE_LOG(LogTemp, Display, TEXT("%s reached maximum momentum!"), *Character->GetName());
	}
}

void ARMCGameMode::SetStartingMomentum(float NewStartingMomentum)
{
	StartingMomentum = FMath::Max(0.0f, NewStartingMomentum);

	// If game has already started, apply to existing characters
	if (bGameStarted)
	{
		TArray<AActor*> FoundCharacters;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARMCCharacter::StaticClass(), FoundCharacters);

		for (AActor* Actor : FoundCharacters)
		{
			ARMCCharacter* Character = Cast<ARMCCharacter>(Actor);
			if (Character)
			{
				ApplyStartingMomentumToCharacter(Character);
			}
		}
	}
}

void ARMCGameMode::ToggleDebugMode()
{
	bDebugModeEnabled = !bDebugModeEnabled;

	// Update all player controllers
	TArray<AActor*> FoundControllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARMCPlayerController::StaticClass(), FoundControllers);

	for (AActor* Actor : FoundControllers)
	{
		ARMCPlayerController* Controller = Cast<ARMCPlayerController>(Actor);
		if (Controller)
		{
			Controller->bShowDebugInfo = bDebugModeEnabled;
		}
	}
}

bool ARMCGameMode::IsDebugModeEnabled() const
{
	return bDebugModeEnabled;
}

void ARMCGameMode::ApplyStartingMomentumToCharacter(ARMCCharacter* Character)
{
	if (Character && StartingMomentum > 0.0f)
	{
		URMCMovementComponent* MovementComponent = Character->GetRMCMovementComponent();
		if (MovementComponent)
		{
			// Set initial momentum
			MovementComponent->CurrentMomentum = StartingMomentum;
			
			// Trigger momentum changed event
			MovementComponent->OnMomentumChanged.Broadcast(StartingMomentum);
			
			// Check if we're starting at max momentum
			if (StartingMomentum >= MovementComponent->MaxMomentum)
			{
				OnPlayerReachedMaxMomentum(Character);
			}
		}
	}
}

void ARMCGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Call parent implementation first
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// Setup player defaults
	if (NewPlayer)
	{
		// Set debug mode on controller
		ARMCPlayerController* RMCController = Cast<ARMCPlayerController>(NewPlayer);
		if (RMCController)
		{
			RMCController->bShowDebugInfo = bDebugModeEnabled;
		}

		// Apply starting momentum to character
		APawn* Pawn = NewPlayer->GetPawn();
		if (Pawn)
		{
			ARMCCharacter* Character = Cast<ARMCCharacter>(Pawn);
			if (Character)
			{
				SetupPlayerDefaults(Character);
			}
		}
	}
}

void ARMCGameMode::SetupPlayerDefaults(ARMCCharacter* Character)
{
	if (Character)
	{
		// Apply starting momentum
		ApplyStartingMomentumToCharacter(Character);

		// Subscribe to momentum changes to detect max momentum
		URMCMovementComponent* MovementComponent = Character->GetRMCMovementComponent();
		if (MovementComponent)
		{
			// Store the character reference
			MomentumCharacter = Character;
			
			// Create a dynamic delegate to check for max momentum when momentum changes
			MovementComponent->OnMomentumChanged.AddDynamic(this, &ARMCGameMode::CheckForMaxMomentum);
		}
	}
}

void ARMCGameMode::CheckForMaxMomentum(float NewMomentum)
{
	// Get the character that triggered this momentum change
	ARMCCharacter* Character = MomentumCharacter.Get();
	if (Character)
	{
		URMCMovementComponent* MovementComponent = Character->GetRMCMovementComponent();
		if (MovementComponent && NewMomentum >= MovementComponent->MaxMomentum)
		{
			OnPlayerReachedMaxMomentum(Character);
		}
	}
}