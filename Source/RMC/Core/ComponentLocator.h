// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComponentLocator.generated.h"

/**
 * Component that provides a service locator for component discovery.
 * This allows components to find each other without direct references.
 * 
 * Usage:
 * 1. Add a ComponentLocator to your actor
 * 2. Register components with the locator in their OnRegister method
 * 3. Unregister components in their OnUnregister method
 * 4. Use the locator to find components by name or class
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UComponentLocator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UComponentLocator();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Registers a component with the locator.
	 * 
	 * @param Component The component to register
	 * @param Name The name to register the component under
	 */
	UFUNCTION(BlueprintCallable, Category = "Component Locator")
	void RegisterComponent(UActorComponent* Component, const FString& Name);

	/**
	 * Unregisters a component from the locator.
	 * 
	 * @param Name The name the component was registered under
	 */
	UFUNCTION(BlueprintCallable, Category = "Component Locator")
	void UnregisterComponent(const FString& Name);

	/**
	 * Gets a component by name.
	 * 
	 * @param Name The name the component was registered under
	 * @return The component, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Component Locator")
	UActorComponent* GetComponentByName(const FString& Name) const;

	/**
	 * Gets a component by class.
	 * 
	 * @param ComponentClass The class of the component to find
	 * @return The component, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Component Locator")
	UActorComponent* GetComponentByClass(TSubclassOf<UActorComponent> ComponentClass) const;

	/**
	 * Gets all components of a specific class.
	 * 
	 * @param ComponentClass The class of the components to find
	 * @param OutComponents Array to store the found components
	 */
	UFUNCTION(BlueprintCallable, Category = "Component Locator")
	void GetComponentsByClass(TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& OutComponents) const;

	/**
	 * Checks if a component is registered under a specific name.
	 * 
	 * @param Name The name to check
	 * @return True if a component is registered under the name, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Component Locator")
	bool HasComponentWithName(const FString& Name) const;

	/**
	 * Checks if a component of a specific class is registered.
	 * 
	 * @param ComponentClass The class to check
	 * @return True if a component of the class is registered, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Component Locator")
	bool HasComponentOfClass(TSubclassOf<UActorComponent> ComponentClass) const;

	/**
	 * Gets a component by name and casts it to the specified type.
	 * 
	 * @param Name The name the component was registered under
	 * @return The component cast to the specified type, or nullptr if not found or cast fails
	 */
	template<class T>
	T* GetComponentByNameAs(const FString& Name) const;

	/**
	 * Gets a component by class and casts it to the specified type.
	 * 
	 * @return The component cast to the specified type, or nullptr if not found
	 */
	template<class T>
	T* GetComponentByClassAs() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called when the component is being destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Map of component names to components
	TMap<FString, UActorComponent*> RegisteredComponents;

	// Map of component classes to components
	TMap<UClass*, UActorComponent*> ClassComponents;

	// Map of component classes to arrays of components
	TMap<UClass*, TArray<UActorComponent*>> ClassComponentArrays;

	// Helper function to update class maps
	void UpdateClassMaps(UActorComponent* Component, bool bAdd);
};

// Template implementation
template<class T>
T* UComponentLocator::GetComponentByNameAs(const FString& Name) const
{
	UActorComponent* Component = GetComponentByName(Name);
	return Cast<T>(Component);
}

template<class T>
T* UComponentLocator::GetComponentByClassAs() const
{
	UActorComponent* Component = GetComponentByClass(T::StaticClass());
	return Cast<T>(Component);
}