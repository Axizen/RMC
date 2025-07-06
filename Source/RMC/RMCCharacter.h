// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Core/GameEventSystem.h"
#include "RMCCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class URiftComponent;
class UWeaponManagerComponent;
class UStyleComponent;
class UMomentumComponent;
class UWallRunComponent;
class UComponentLocator;
class UGameEventSubsystem;
class UInputHandlerComponent;
class ARiftAnchor;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ARMCCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** Component Locator */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
	UComponentLocator* ComponentLocator;

	/** Input Handler Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
	UInputHandlerComponent* InputHandler;

	/** Rift Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rift", meta = (AllowPrivateAccess = "true"))
	URiftComponent* RiftComponent;

	/** Weapon Manager Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UWeaponManagerComponent* WeaponManager;

	/** Style Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UStyleComponent* StyleComponent;

	/** Momentum Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UMomentumComponent* MomentumComponent;

	/** Wall Run Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	UWallRunComponent* WallRunComponent;

	/** Double Jump Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	class UDoubleJumpComponent* DoubleJumpComponent;

	/** Normal gameplay input context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* NormalGameplayContext;

	/** Combat input context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* CombatContext;

	/** Rifting input context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* RiftingContext;

	/** Wall running input context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* WallRunningContext;

	/** Menu input context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MenuContext;

public:
	ARMCCharacter();
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns ComponentLocator subobject **/
	FORCEINLINE class UComponentLocator* GetComponentLocator() const { return ComponentLocator; }
	/** Returns InputHandler subobject **/
	FORCEINLINE class UInputHandlerComponent* GetInputHandler() const { return InputHandler; }
	/** Returns RiftComponent subobject **/
	FORCEINLINE class URiftComponent* GetRiftComponent() const { return RiftComponent; }
	/** Returns WeaponManager subobject **/
	FORCEINLINE class UWeaponManagerComponent* GetWeaponManager() const { return WeaponManager; }
	/** Returns StyleComponent subobject **/
	FORCEINLINE class UStyleComponent* GetStyleComponent() const { return StyleComponent; }
	/** Returns MomentumComponent subobject **/
	FORCEINLINE class UMomentumComponent* GetMomentumComponent() const { return MomentumComponent; }
	/** Returns WallRunComponent subobject **/
	FORCEINLINE class UWallRunComponent* GetWallRunComponent() const { return WallRunComponent; }
	/** Returns DoubleJumpComponent subobject **/
	FORCEINLINE class UDoubleJumpComponent* GetDoubleJumpComponent() const { return DoubleJumpComponent; }

	/** Add style points from a specific move */
	UFUNCTION(BlueprintCallable, Category = "Combat|Style")
	void AddStylePoints(float Points, FName MoveName);

	/** Handle taking damage and update style accordingly */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** Called when the game ends */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** Input action for double jump */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DoubleJumpAction;

	/** Called for movement input */
	UFUNCTION()
	void OnMoveInput(const FGameEventData& EventData);

	/** Called for looking input */
	UFUNCTION()
	void OnLookInput(const FGameEventData& EventData);

	/** Called for jump input */
	UFUNCTION()
	void OnJumpInput(const FGameEventData& EventData);

	/** Called for phantom dodge input */
	UFUNCTION()
	void OnPhantomDodgeInput(const FGameEventData& EventData);

	/** Called for rift tether input */
	UFUNCTION()
	void OnRiftTetherInput(const FGameEventData& EventData);

	/** Called for weapon fire input */
	UFUNCTION()
	void OnFireInput(const FGameEventData& EventData);

	/** Called for weapon alt fire input */
	UFUNCTION()
	void OnAltFireInput(const FGameEventData& EventData);

	/** Called when releasing alt fire input */
	UFUNCTION()
	void OnReleaseChargeInput(const FGameEventData& EventData);

	/** Called for weapon reload input */
	UFUNCTION()
	void OnReloadInput(const FGameEventData& EventData);

	/** Called for weapon switch input */
	UFUNCTION()
	void OnWeaponSwitchInput(const FGameEventData& EventData);

	/** Called for wall run input */
	UFUNCTION()
	void OnWallRunInput(const FGameEventData& EventData);

	/** Called for wall jump input */
	UFUNCTION()
	void OnWallJumpInput(const FGameEventData& EventData);

	/** Called for double jump input */
	UFUNCTION()
	void TryDoubleJump(const FInputActionValue& Value);

	/** Find the best rift anchor in range */
	ARiftAnchor* FindBestRiftAnchor() const;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;

	/** Default weapons to equip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<TSubclassOf<class URangedWeaponBase>> DefaultWeapons;

	// Event system reference
	UGameEventSubsystem* EventSystem;

	// Event listener handles
	TArray<FDelegateHandle> EventListenerHandles;

	// Event handlers for gameplay events
	UFUNCTION()
	void OnMomentumChanged(const FGameEventData& EventData);

	UFUNCTION()
	void OnStylePointsGained(const FGameEventData& EventData);

	UFUNCTION()
	void OnRiftPerformed(const FGameEventData& EventData);

	UFUNCTION()
	void OnWallRunStarted(const FGameEventData& EventData);

	UFUNCTION()
	void OnWallRunEnded(const FGameEventData& EventData);

	UFUNCTION()
	void OnWeaponFired(const FGameEventData& EventData);

	// Note: We're temporarily commenting out these interface methods due to linker errors
	// We'll use direct component references for now
	/*
	class IMomentumInterface* GetMomentumInterface() const;
	class IRiftInterface* GetRiftInterface() const;
	class IStyleInterface* GetStyleInterface() const;
	class IWallRunInterface* GetWallRunInterface() const;
	*/

	// Helper method to broadcast events
	void BroadcastGameEvent(EGameEventType EventType, float FloatValue = 0.0f, int32 IntValue = 0, FName NameValue = NAME_None, AActor* Target = nullptr);

	// Setup input actions for the InputHandlerComponent
	void SetupInputActions();
};