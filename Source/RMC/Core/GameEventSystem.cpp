// Fill out your copyright notice in the Description page of Project Settings.

#include "GameEventSystem.h"

void UGameEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize event listener maps
	EventListeners.Empty();
	BlueprintEventListeners.Empty();
	HandleToEventType.Empty();

	UE_LOG(LogTemp, Log, TEXT("GameEventSubsystem initialized"));
}

void UGameEventSubsystem::Deinitialize()
{
	// Clear all event listeners
	EventListeners.Empty();
	BlueprintEventListeners.Empty();
	HandleToEventType.Empty();

	UE_LOG(LogTemp, Log, TEXT("GameEventSubsystem deinitialized"));

	Super::Deinitialize();
}

void UGameEventSubsystem::BroadcastEvent(const FGameEventData& EventData)
{
	// Get the array of listeners for this event type
	TArray<TPair<FDelegateHandle, FScriptDelegate>>* Listeners = EventListeners.Find(EventData.EventType);
	if (Listeners)
	{
		// Create a copy of the array to avoid issues if listeners are added or removed during broadcast
		TArray<TPair<FDelegateHandle, FScriptDelegate>> ListenersCopy = *Listeners;

		// Broadcast to all listeners
		for (const TPair<FDelegateHandle, FScriptDelegate>& Listener : ListenersCopy)
		{
			// Check if the listener is still valid
			UObject* Object = Listener.Value.GetUObject();
			if (Object && IsValid(Object))
			{
				// Execute the delegate
				Listener.Value.ProcessDelegate<UObject>(&EventData);
			}
			else
			{
				// Remove invalid listener
				RemoveEventListener(Listener.Key);
			}
		}
	}

	// Get the array of Blueprint listeners for this event type
	TArray<TPair<UObject*, FGameEventDelegate>>* BlueprintListeners = BlueprintEventListeners.Find(EventData.EventType);
	if (BlueprintListeners)
	{
		// Create a copy of the array to avoid issues if listeners are added or removed during broadcast
		TArray<TPair<UObject*, FGameEventDelegate>> BlueprintListenersCopy = *BlueprintListeners;

		// Broadcast to all Blueprint listeners
		for (const TPair<UObject*, FGameEventDelegate>& Listener : BlueprintListenersCopy)
		{
			// Check if the listener is still valid
			if (Listener.Key && IsValid(Listener.Key))
			{
				// Execute the delegate
				Listener.Value.ExecuteIfBound(EventData);
			}
			else
			{
				// Remove invalid listener
				BlueprintEventListeners[EventData.EventType].Remove(Listener);
			}
		}
	}
}

FDelegateHandle UGameEventSubsystem::AddEventListener(UObject* Listener, EGameEventType EventType, FName FunctionName)
{
	// Check if the listener and function are valid
	if (!Listener || !DoesFunctionExist(Listener, FunctionName))
	{
		UE_LOG(LogTemp, Warning, TEXT("AddEventListener: Invalid listener or function: %s"), *FunctionName.ToString());
		return FDelegateHandle();
	}

	// Create a script delegate for the function
	FScriptDelegate Delegate = CreateScriptDelegate(Listener, FunctionName);

	// Generate a unique handle for this listener
	FDelegateHandle Handle = FDelegateHandle(FDelegateHandle::EGenerateNewHandle::GenerateNewHandle);

	// Add the listener to the map
	if (!EventListeners.Contains(EventType))
	{
		EventListeners.Add(EventType, TArray<TPair<FDelegateHandle, FScriptDelegate>>());
	}
	EventListeners[EventType].Add(TPair<FDelegateHandle, FScriptDelegate>(Handle, Delegate));

	// Add the handle to the lookup map
	HandleToEventType.Add(Handle, EventType);

	return Handle;
}

void UGameEventSubsystem::RemoveEventListener(FDelegateHandle Handle)
{
	// Find the event type for this handle
	EGameEventType* EventType = HandleToEventType.Find(Handle);
	if (!EventType)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEventListener: Invalid handle"));
		return;
	}

	// Find the array of listeners for this event type
	TArray<TPair<FDelegateHandle, FScriptDelegate>>* Listeners = EventListeners.Find(*EventType);
	if (!Listeners)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEventListener: No listeners for event type"));
		return;
	}

	// Find and remove the listener with this handle
	for (int32 i = 0; i < Listeners->Num(); i++)
	{
		if ((*Listeners)[i].Key == Handle)
		{
			Listeners->RemoveAt(i);
			break;
		}
	}

	// Remove the handle from the lookup map
	HandleToEventType.Remove(Handle);
}

void UGameEventSubsystem::RemoveAllEventListeners(UObject* Listener)
{
	if (!Listener)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveAllEventListeners: Invalid listener"));
		return;
	}

	// Remove all C++ event listeners
	TArray<FDelegateHandle> HandlesToRemove;

	// Find all handles for this listener
	for (auto& EventListenerPair : EventListeners)
	{
		TArray<TPair<FDelegateHandle, FScriptDelegate>>& Listeners = EventListenerPair.Value;

		for (int32 i = Listeners.Num() - 1; i >= 0; i--)
		{
			if (Listeners[i].Value.GetUObject() == Listener)
			{
				HandlesToRemove.Add(Listeners[i].Key);
				Listeners.RemoveAt(i);
			}
		}
	}

	// Remove handles from the lookup map
	for (const FDelegateHandle& Handle : HandlesToRemove)
	{
		HandleToEventType.Remove(Handle);
	}

	// Remove all Blueprint event listeners
	for (auto& BlueprintEventListenerPair : BlueprintEventListeners)
	{
		TArray<TPair<UObject*, FGameEventDelegate>>& Listeners = BlueprintEventListenerPair.Value;

		for (int32 i = Listeners.Num() - 1; i >= 0; i--)
		{
			if (Listeners[i].Key == Listener)
			{
				Listeners.RemoveAt(i);
			}
		}
	}
}

void UGameEventSubsystem::AddBlueprintEventListener(UObject* Listener, EGameEventType EventType, FGameEventDelegate Callback)
{
	// Check if the listener is valid
	if (!Listener)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddBlueprintEventListener: Invalid listener"));
		return;
	}

	// Add the listener to the map
	if (!BlueprintEventListeners.Contains(EventType))
	{
		BlueprintEventListeners.Add(EventType, TArray<TPair<UObject*, FGameEventDelegate>>());
	}

	// Check if this listener is already registered for this event type
	TArray<TPair<UObject*, FGameEventDelegate>>& Listeners = BlueprintEventListeners[EventType];
	for (int32 i = 0; i < Listeners.Num(); i++)
	{
		if (Listeners[i].Key == Listener)
		{
			// Update the callback
			Listeners[i].Value = Callback;
			return;
		}
	}

	// Add the new listener
	Listeners.Add(TPair<UObject*, FGameEventDelegate>(Listener, Callback));
}

void UGameEventSubsystem::RemoveBlueprintEventListener(UObject* Listener, EGameEventType EventType)
{
	// Check if the listener is valid
	if (!Listener)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveBlueprintEventListener: Invalid listener"));
		return;
	}

	// Find the array of listeners for this event type
	TArray<TPair<UObject*, FGameEventDelegate>>* Listeners = BlueprintEventListeners.Find(EventType);
	if (!Listeners)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveBlueprintEventListener: No listeners for event type"));
		return;
	}

	// Find and remove the listener
	for (int32 i = 0; i < Listeners->Num(); i++)
	{
		if ((*Listeners)[i].Key == Listener)
		{
			Listeners->RemoveAt(i);
			break;
		}
	}
}

FScriptDelegate UGameEventSubsystem::CreateScriptDelegate(UObject* Listener, FName FunctionName)
{
	FScriptDelegate Delegate;
	Delegate.BindUFunction(Listener, FunctionName);
	return Delegate;
}

bool UGameEventSubsystem::DoesFunctionExist(UObject* Object, FName FunctionName)
{
	if (!Object)
	{
		return false;
	}

	UFunction* Function = Object->FindFunction(FunctionName);
	if (!Function)
	{
		return false;
	}

	// Check if the function has the correct signature
	// It should have one parameter of type FGameEventData
	TFieldIterator<FProperty> PropIt(Function);
	if (!PropIt)
	{
		return false;
	}

	FProperty* Prop = *PropIt;
	if (!Prop || !Prop->IsA<FStructProperty>())
	{
		return false;
	}

	FStructProperty* StructProp = CastField<FStructProperty>(Prop);
	if (!StructProp || StructProp->Struct != FGameEventData::StaticStruct())
	{
		return false;
	}

	// Make sure there's only one parameter
	++PropIt;
	if (PropIt)
	{
		return false;
	}

	return true;
}