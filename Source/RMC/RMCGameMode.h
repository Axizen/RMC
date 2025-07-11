// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RMCGameMode.generated.h"

/**
 * Game mode for the RMC game with momentum-based movement system
 */
UCLASS(Blueprintable, BlueprintType, meta=(ShortTooltip="Game mode for momentum-based movement system."))
class RMC_API ARMCGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// Constructor
	ARMCGameMode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Debug settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta=(ToolTip="Enable debug features and logging"))
	bool bDebugModeEnabled;

	// Momentum settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Momentum", meta=(ToolTip="Starting momentum value for players", ClampMin="0.0"))
	float StartingMomentum;

	// Blueprint events
	/** Called when the game starts */
	UFUNCTION(BlueprintNativeEvent, Category = "Game Events", meta=(ToolTip="Called when the game starts"))
	void OnGameStart();
	virtual void OnGameStart_Implementation();

	/** Called when a player reaches maximum momentum */
	UFUNCTION(BlueprintNativeEvent, Category = "Game Events", meta=(ToolTip="Called when a player reaches maximum momentum"))
	void OnPlayerReachedMaxMomentum(class ARMCCharacter* Character);
	virtual void OnPlayerReachedMaxMomentum_Implementation(class ARMCCharacter* Character);

	// Blueprint callable functions
	/** Sets the starting momentum for all players */
	UFUNCTION(BlueprintCallable, Category = "Momentum", meta=(ToolTip="Sets the starting momentum for all players"))
	void SetStartingMomentum(float NewStartingMomentum);

	/** Toggles debug mode */
	UFUNCTION(BlueprintCallable, Category = "Debug", meta=(ToolTip="Toggles debug mode on/off"))
	void ToggleDebugMode();

	/** Gets whether debug mode is enabled */
	UFUNCTION(BlueprintPure, Category = "Debug", meta=(ToolTip="Returns whether debug mode is enabled"))
	bool IsDebugModeEnabled() const;

	/** Applies starting momentum to a character */
	UFUNCTION(BlueprintCallable, Category = "Momentum", meta=(ToolTip="Applies the configured starting momentum to a character"))
	void ApplyStartingMomentumToCharacter(class ARMCCharacter* Character);

	/** Called when a new player joins the game */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

protected:
	// Helper functions
	void SetupPlayerDefaults(class ARMCCharacter* Character);
	
	/** Checks if a character has reached max momentum */
	UFUNCTION()
	void CheckForMaxMomentum(float NewMomentum);
	
	// Flag to track if game has started
	bool bGameStarted;
	
	// Reference to the character that triggered the momentum change
	TWeakObjectPtr<ARMCCharacter> MomentumCharacter;
};