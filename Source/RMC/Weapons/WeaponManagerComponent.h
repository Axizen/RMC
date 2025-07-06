// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Core/MomentumInterface.h"
#include "../Core/RiftInterface.h"
#include "../Core/StyleInterface.h"
#include "../Core/GameEventSystem.h"
#include "WeaponManagerComponent.generated.h"

class URangedWeaponBase;
class UGameEventSubsystem;
class URiftComponent;
class UMomentumComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponManagerComponent();

	// Called when the component is registered
	virtual void OnRegister() override;

	// Called when the component is unregistered
	virtual void OnUnregister() override;

	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Weapon inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TArray<URangedWeaponBase*> WeaponInventory;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	URangedWeaponBase* CurrentWeapon;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapons")
	int32 CurrentWeaponIndex;

	// Maximum number of weapons that can be carried
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	int32 MaxWeapons;
	
	// Functions
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void SwitchToWeapon(int32 WeaponIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void NextWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void PreviousWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool AddWeaponToInventory(TSubclassOf<URangedWeaponBase> WeaponClass);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool RemoveWeaponFromInventory(int32 WeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	bool HasWeapon(TSubclassOf<URangedWeaponBase> WeaponClass) const;

	UFUNCTION(BlueprintPure, Category = "Weapons")
	int32 GetWeaponCount() const { return WeaponInventory.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void FireCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void AltFireCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void StartReloadCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void StartChargeCurrentWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	void ReleaseChargeCurrentWeapon();
	
	// Event handlers
	UFUNCTION()
	void OnRiftStateChanged(const FGameEventData& EventData);

	// Legacy rift integration (for backward compatibility)
	UFUNCTION()
	void OnRiftBegin();
	
	UFUNCTION()
	void OnRiftEnd();

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponSwitched, URangedWeaponBase*, NewWeapon, URangedWeaponBase*, PreviousWeapon);
	UPROPERTY(BlueprintAssignable, Category = "Weapons|Events")
	FOnWeaponSwitched OnWeaponSwitched;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAdded, URangedWeaponBase*, NewWeapon);
	UPROPERTY(BlueprintAssignable, Category = "Weapons|Events")
	FOnWeaponAdded OnWeaponAdded;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponRemoved, URangedWeaponBase*, RemovedWeapon);
	UPROPERTY(BlueprintAssignable, Category = "Weapons|Events")
	FOnWeaponRemoved OnWeaponRemoved;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Event system reference
	UPROPERTY()
	UGameEventSubsystem* EventSystem;

	// Event listener handles
	TArray<int32> EventListenerHandles;

	// Broadcast weapon fired event
	void BroadcastWeaponFiredEvent(URangedWeaponBase* Weapon, bool bSuccess);

	// Broadcast weapon switched event
	void BroadcastWeaponSwitchedEvent(URangedWeaponBase* NewWeapon, URangedWeaponBase* PreviousWeapon);

	// Broadcast weapon reloaded event
	void BroadcastWeaponReloadedEvent(URangedWeaponBase* Weapon);

	// Helper functions to get interfaces
	TScriptInterface<IMomentumInterface> GetMomentumInterface() const;
	TScriptInterface<IRiftInterface> GetRiftInterface() const;
	TScriptInterface<IStyleInterface> GetStyleInterface() const;

	// Legacy helper functions (for backward compatibility)
	URiftComponent* GetOwnerRiftComponent() const;
	UMomentumComponent* GetOwnerMomentumComponent() const;
};