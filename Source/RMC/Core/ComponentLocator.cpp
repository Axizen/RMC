// Fill out your copyright notice in the Description page of Project Settings.

#include "ComponentLocator.h"

// Sets default values for this component's properties
UComponentLocator::UComponentLocator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = false;

	// Initialize maps
	RegisteredComponents.Empty();
	ClassComponents.Empty();
	ClassComponentArrays.Empty();
}

// Called when the game starts
void UComponentLocator::BeginPlay()
{
	Super::BeginPlay();

	// Register all components on the owner
	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (Component && Component != this)
		{
			// Register the component with its class name
			FString ClassName = Component->GetClass()->GetName();
			RegisterComponent(Component, ClassName);
		}
	}
}

// Called when the component is being destroyed
void UComponentLocator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear all registered components
	RegisteredComponents.Empty();
	ClassComponents.Empty();
	ClassComponentArrays.Empty();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UComponentLocator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// No need for tick functionality
}

void UComponentLocator::RegisterComponent(UActorComponent* Component, const FString& Name)
{
	if (!Component)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterComponent: Invalid component"));
		return;
	}

	// Register the component by name
	RegisteredComponents.Add(Name, Component);

	// Update class maps
	UpdateClassMaps(Component, true);

	UE_LOG(LogTemp, Verbose, TEXT("Registered component %s with name %s"), *Component->GetName(), *Name);
}

void UComponentLocator::UnregisterComponent(const FString& Name)
{
	// Find the component
	UActorComponent** ComponentPtr = RegisteredComponents.Find(Name);
	if (!ComponentPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UnregisterComponent: No component registered with name %s"), *Name);
		return;
	}

	UActorComponent* Component = *ComponentPtr;

	// Update class maps
	UpdateClassMaps(Component, false);

	// Remove the component from the name map
	RegisteredComponents.Remove(Name);

	UE_LOG(LogTemp, Verbose, TEXT("Unregistered component with name %s"), *Name);
}

UActorComponent* UComponentLocator::GetComponentByName(const FString& Name) const
{
	// Find the component
	UActorComponent* const* ComponentPtr = RegisteredComponents.Find(Name);
	if (!ComponentPtr)
	{
		return nullptr;
	}

	return *ComponentPtr;
}

UActorComponent* UComponentLocator::GetComponentByClass(TSubclassOf<UActorComponent> ComponentClass) const
{
	if (!ComponentClass)
	{
		return nullptr;
	}

	// Try to find an exact match first
	UActorComponent* const* ExactMatch = ClassComponents.Find(ComponentClass);
	if (ExactMatch)
	{
		return *ExactMatch;
	}

	// If no exact match, look for a component that is a subclass of the requested class
	for (const auto& Pair : ClassComponents)
	{
		if (Pair.Key->IsChildOf(ComponentClass))
		{
			return Pair.Value;
		}
	}

	return nullptr;
}

void UComponentLocator::GetComponentsByClass(TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& OutComponents) const
{
	OutComponents.Empty();

	if (!ComponentClass)
	{
		return;
	}

	// Try to find an exact match first
	const TArray<UActorComponent*>* ExactMatches = ClassComponentArrays.Find(ComponentClass);
	if (ExactMatches)
	{
		OutComponents.Append(*ExactMatches);
	}

	// Also look for components that are subclasses of the requested class
	for (const auto& Pair : ClassComponentArrays)
	{
		if (Pair.Key != ComponentClass && Pair.Key->IsChildOf(ComponentClass))
		{
			OutComponents.Append(Pair.Value);
		}
	}
}

bool UComponentLocator::HasComponentWithName(const FString& Name) const
{
	return RegisteredComponents.Contains(Name);
}

bool UComponentLocator::HasComponentOfClass(TSubclassOf<UActorComponent> ComponentClass) const
{
	if (!ComponentClass)
	{
		return false;
	}

	// Check for exact match
	if (ClassComponents.Contains(ComponentClass))
	{
		return true;
	}

	// Check for subclass match
	for (const auto& Pair : ClassComponents)
	{
		if (Pair.Key->IsChildOf(ComponentClass))
		{
			return true;
		}
	}

	return false;
}

void UComponentLocator::UpdateClassMaps(UActorComponent* Component, bool bAdd)
{
	if (!Component)
	{
		return;
	}

	UClass* ComponentClass = Component->GetClass();

	if (bAdd)
	{
		// Add to class map
		ClassComponents.Add(ComponentClass, Component);

		// Add to class array map
		if (!ClassComponentArrays.Contains(ComponentClass))
		{
			ClassComponentArrays.Add(ComponentClass, TArray<UActorComponent*>());
		}
		ClassComponentArrays[ComponentClass].Add(Component);

		// Also add to parent class arrays
		UClass* ParentClass = ComponentClass->GetSuperClass();
		while (ParentClass && ParentClass != UActorComponent::StaticClass())
		{
			if (!ClassComponentArrays.Contains(ParentClass))
			{
				ClassComponentArrays.Add(ParentClass, TArray<UActorComponent*>());
			}
			ClassComponentArrays[ParentClass].Add(Component);

			ParentClass = ParentClass->GetSuperClass();
		}
	}
	else
	{
		// Remove from class map
		ClassComponents.Remove(ComponentClass);

		// Remove from class array map
		if (ClassComponentArrays.Contains(ComponentClass))
		{
			ClassComponentArrays[ComponentClass].Remove(Component);
			if (ClassComponentArrays[ComponentClass].Num() == 0)
			{
				ClassComponentArrays.Remove(ComponentClass);
			}
		}

		// Also remove from parent class arrays
		UClass* ParentClass = ComponentClass->GetSuperClass();
		while (ParentClass && ParentClass != UActorComponent::StaticClass())
		{
			if (ClassComponentArrays.Contains(ParentClass))
			{
				ClassComponentArrays[ParentClass].Remove(Component);
				if (ClassComponentArrays[ParentClass].Num() == 0)
				{
					ClassComponentArrays.Remove(ParentClass);
				}
			}

			ParentClass = ParentClass->GetSuperClass();
		}
	}
}