// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeWeaponManagerComponent.h"
#include "MeleeWeaponBase.h"
#include "WeaponManagerComponent.h"
#include "StyleComponent.h"
#include "../MomentumComponent.h"
#include "../Rift/RiftComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UMeleeWeaponManagerComponent::UMeleeWeaponManagerComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = true;

    // Initialize properties
    CurrentMeleeWeaponIndex = 0;
    MaxMeleeWeapons = 4;
    CurrentCombatStyle = ECombatStyle::None;
    bInMeleeStance = true;
    InputBufferTimeWindow = 0.5f;
    
    // Initialize unlocked styles
    UnlockedStyles.Add(ECombatStyle::Swordmaster); // Start with Swordmaster unlocked
}

// Called when the game starts
void UMeleeWeaponManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get references to other components
    StyleComponent = GetOwnerStyleComponent();
    MomentumComponent = GetOwnerMomentumComponent();
    RiftComponent = GetOwnerRiftComponent();
    WeaponManagerComponent = GetOwnerWeaponManagerComponent();
    
    // Set up rift event bindings
    if (RiftComponent)
    {
        RiftComponent->OnRiftBegin.AddDynamic(this, &UMeleeWeaponManagerComponent::OnRiftBegin);
        RiftComponent->OnRiftEnd.AddDynamic(this, &UMeleeWeaponManagerComponent::OnRiftEnd);
    }
    
    // Initialize with first weapon if available
    if (MeleeWeaponInventory.Num() > 0)
    {
        SwitchToMeleeWeapon(0);
    }
    
    // Start in melee stance by default
    SwitchToMeleeStance();
    
    // Initialize style mastery levels if empty
    if (StyleMasteryLevels.Num() == 0)
    {
        for (ECombatStyle Style : UnlockedStyles)
        {
            StyleMasteryLevels.Add(Style, 1); // Start at level 1
        }
    }
}

// Called every frame
void UMeleeWeaponManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Process input buffer cleanup
    if (InputBuffer.Num() > 0)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        TArray<FBufferedInput> ValidInputs;
        
        for (const FBufferedInput& Input : InputBuffer)
        {
            if (CurrentTime - Input.TimeStamp <= InputBufferTimeWindow)
            {
                ValidInputs.Add(Input);
            }
        }
        
        InputBuffer = ValidInputs;
    }
}

// Switch to a specific melee weapon by index
void UMeleeWeaponManagerComponent::SwitchToMeleeWeapon(int32 WeaponIndex)
{
    if (!MeleeWeaponInventory.IsValidIndex(WeaponIndex) || WeaponIndex == CurrentMeleeWeaponIndex)
    {
        return;
    }
    
    // Store the previous weapon
    UMeleeWeaponBase* PreviousWeapon = CurrentMeleeWeapon;
    
    // Update current weapon index
    CurrentMeleeWeaponIndex = WeaponIndex;
    CurrentMeleeWeapon = MeleeWeaponInventory[CurrentMeleeWeaponIndex];
    
    // Broadcast weapon switched event
    OnMeleeWeaponSwitched.Broadcast(CurrentMeleeWeapon, PreviousWeapon);
}

// Switch to the next melee weapon in the inventory
void UMeleeWeaponManagerComponent::NextMeleeWeapon()
{
    if (MeleeWeaponInventory.Num() <= 1)
    {
        return;
    }
    
    int32 NextIndex = (CurrentMeleeWeaponIndex + 1) % MeleeWeaponInventory.Num();
    SwitchToMeleeWeapon(NextIndex);
}

// Switch to the previous melee weapon in the inventory
void UMeleeWeaponManagerComponent::PreviousMeleeWeapon()
{
    if (MeleeWeaponInventory.Num() <= 1)
    {
        return;
    }
    
    int32 PrevIndex = (CurrentMeleeWeaponIndex - 1 + MeleeWeaponInventory.Num()) % MeleeWeaponInventory.Num();
    SwitchToMeleeWeapon(PrevIndex);
}

// Add a melee weapon to the inventory
bool UMeleeWeaponManagerComponent::AddMeleeWeaponToInventory(TSubclassOf<UMeleeWeaponBase> WeaponClass)
{
    if (!WeaponClass || MeleeWeaponInventory.Num() >= MaxMeleeWeapons)
    {
        return false;
    }
    
    // Check if we already have this weapon type
    for (UMeleeWeaponBase* Weapon : MeleeWeaponInventory)
    {
        if (Weapon && Weapon->GetClass() == WeaponClass)
        {
            return false; // Already have this weapon
        }
    }
    
    // Create the weapon component
    UMeleeWeaponBase* NewWeapon = NewObject<UMeleeWeaponBase>(GetOwner(), WeaponClass);
    if (NewWeapon)
    {
        NewWeapon->RegisterComponent();
        MeleeWeaponInventory.Add(NewWeapon);
        
        // If this is our first weapon, switch to it
        if (MeleeWeaponInventory.Num() == 1)
        {
            SwitchToMeleeWeapon(0);
        }
        
        // Broadcast weapon added event
        OnMeleeWeaponAdded.Broadcast(NewWeapon);
        
        return true;
    }
    
    return false;
}

// Remove a melee weapon from the inventory
bool UMeleeWeaponManagerComponent::RemoveMeleeWeaponFromInventory(int32 WeaponIndex)
{
    if (!MeleeWeaponInventory.IsValidIndex(WeaponIndex))
    {
        return false;
    }
    
    // Store the weapon being removed
    UMeleeWeaponBase* RemovedWeapon = MeleeWeaponInventory[WeaponIndex];
    
    // Remove the weapon from the inventory
    MeleeWeaponInventory.RemoveAt(WeaponIndex);
    
    // If we removed the current weapon, switch to another one
    if (WeaponIndex == CurrentMeleeWeaponIndex)
    {
        if (MeleeWeaponInventory.Num() > 0)
        {
            // Switch to the first available weapon
            SwitchToMeleeWeapon(0);
        }
        else
        {
            // No weapons left
            CurrentMeleeWeapon = nullptr;
            CurrentMeleeWeaponIndex = 0;
        }
    }
    else if (WeaponIndex < CurrentMeleeWeaponIndex)
    {
        // Adjust current index if we removed a weapon before it
        CurrentMeleeWeaponIndex--;
    }
    
    // Broadcast weapon removed event
    OnMeleeWeaponRemoved.Broadcast(RemovedWeapon);
    
    // Unregister and destroy the removed weapon
    if (RemovedWeapon)
    {
        RemovedWeapon->DestroyComponent();
    }
    
    return true;
}

// Check if the inventory contains a specific weapon class
bool UMeleeWeaponManagerComponent::HasMeleeWeapon(TSubclassOf<UMeleeWeaponBase> WeaponClass) const
{
    for (UMeleeWeaponBase* Weapon : MeleeWeaponInventory)
    {
        if (Weapon && Weapon->GetClass() == WeaponClass)
        {
            return true;
        }
    }
    
    return false;
}

// Switch to a different combat style
void UMeleeWeaponManagerComponent::SwitchCombatStyle(ECombatStyle NewStyle)
{
    if (NewStyle == CurrentCombatStyle || !CanUseCombatStyle(NewStyle))
    {
        return;
    }
    
    ECombatStyle PreviousStyle = CurrentCombatStyle;
    CurrentCombatStyle = NewStyle;
    
    // Switch to appropriate stance based on style
    if (NewStyle == ECombatStyle::Gunslinger)
    {
        SwitchToRangedStance();
    }
    else if (NewStyle == ECombatStyle::Swordmaster)
    {
        SwitchToMeleeStance();
    }
    // Other styles don't automatically change stance
    
    // Broadcast style changed event
    OnCombatStyleChanged.Broadcast(NewStyle);
}

// Check if a combat style can be used
bool UMeleeWeaponManagerComponent::CanUseCombatStyle(ECombatStyle Style) const
{
    return UnlockedStyles.Contains(Style);
}

// Switch to melee stance
void UMeleeWeaponManagerComponent::SwitchToMeleeStance()
{
    if (bInMeleeStance)
    {
        return;
    }
    
    bInMeleeStance = true;
    
    // Notify the ranged weapon manager to holster weapons if available
    if (WeaponManagerComponent)
    {
        // Implementation depends on how you want to handle this
        // For now, we'll just broadcast the stance change
    }
    
    // Broadcast stance changed event
    OnCombatStanceChanged.Broadcast(bInMeleeStance);
}

// Switch to ranged stance
void UMeleeWeaponManagerComponent::SwitchToRangedStance()
{
    if (!bInMeleeStance)
    {
        return;
    }
    
    bInMeleeStance = false;
    
    // End any ongoing melee actions
    if (CurrentMeleeWeapon)
    {
        CurrentMeleeWeapon->ResetCombo();
        CurrentMeleeWeapon->EndGuard();
    }
    
    // Broadcast stance changed event
    OnCombatStanceChanged.Broadcast(bInMeleeStance);
}

// Toggle between melee and ranged stance
void UMeleeWeaponManagerComponent::ToggleCombatStance()
{
    if (bInMeleeStance)
    {
        SwitchToRangedStance();
    }
    else
    {
        SwitchToMeleeStance();
    }
}

// Perform a light attack
bool UMeleeWeaponManagerComponent::PerformLightAttack()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return false;
    }
    
    // Add to input buffer
    ProcessBufferedInput(EAttackInput::Light);
    
    return CurrentMeleeWeapon->PerformAttack(EAttackInput::Light);
}

// Perform a heavy attack
bool UMeleeWeaponManagerComponent::PerformHeavyAttack()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return false;
    }
    
    // Add to input buffer
    ProcessBufferedInput(EAttackInput::Heavy);
    
    return CurrentMeleeWeapon->PerformAttack(EAttackInput::Heavy);
}

// Perform a directional attack
bool UMeleeWeaponManagerComponent::PerformDirectionalAttack(EDirection Direction)
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return false;
    }
    
    // Map direction to attack input
    EAttackInput AttackInput = EAttackInput::None;
    
    switch (Direction)
    {
        case EDirection::Forward:
            AttackInput = EAttackInput::Forward;
            break;
        case EDirection::Backward:
            AttackInput = EAttackInput::Back;
            break;
        case EDirection::Up:
            AttackInput = EAttackInput::Up;
            break;
        case EDirection::Down:
            AttackInput = EAttackInput::Down;
            break;
        default:
            return false;
    }
    
    // Add to input buffer
    ProcessBufferedInput(AttackInput);
    
    return CurrentMeleeWeapon->PerformAttack(AttackInput);
}

// Perform a special attack
bool UMeleeWeaponManagerComponent::PerformSpecialAttack()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return false;
    }
    
    // Add to input buffer
    ProcessBufferedInput(EAttackInput::Special);
    
    return CurrentMeleeWeapon->PerformAttack(EAttackInput::Special);
}

// Attempt to guard/block
bool UMeleeWeaponManagerComponent::AttemptGuard()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return false;
    }
    
    return CurrentMeleeWeapon->AttemptGuard();
}

// End guarding
void UMeleeWeaponManagerComponent::EndGuard()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return;
    }
    
    CurrentMeleeWeapon->EndGuard();
}

// Attempt a jump cancel
bool UMeleeWeaponManagerComponent::AttemptJumpCancel()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return false;
    }
    
    return CurrentMeleeWeapon->AttemptJumpCancel();
}

// Start charging an attack
void UMeleeWeaponManagerComponent::StartChargeAttack()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return;
    }
    
    CurrentMeleeWeapon->ChargeAttack(0.0f);
}

// Release a charged attack
void UMeleeWeaponManagerComponent::ReleaseChargeAttack()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return;
    }
    
    CurrentMeleeWeapon->ReleaseChargeAttack();
}

// Attempt an Exceed timing (DMC-style)
bool UMeleeWeaponManagerComponent::AttemptExceedTiming()
{
    if (!bInMeleeStance || !CurrentMeleeWeapon)
    {
        return false;
    }
    
    // Add to input buffer
    ProcessBufferedInput(EAttackInput::Exceed);
    
    // This will only work if the current weapon supports Exceed
    // We'll check this in the SwordMeleeWeapon implementation
    return CurrentMeleeWeapon->PerformAttack(EAttackInput::Exceed);
}

// Called when a rift begins
void UMeleeWeaponManagerComponent::OnRiftBegin()
{
    // Handle rift-specific behavior for melee weapons
    // For example, enable special attacks or modify properties
}

// Called when a rift ends
void UMeleeWeaponManagerComponent::OnRiftEnd()
{
    // Reset any rift-specific behavior for melee weapons
}

// Process buffered inputs for combo detection
bool UMeleeWeaponManagerComponent::ProcessBufferedInput(EAttackInput NewInput)
{
    // Add the new input to the buffer
    InputBuffer.Add(FBufferedInput(NewInput, GetWorld()->GetTimeSeconds()));
    
    // For now, we're just storing inputs for potential future use
    // A more complex implementation could detect specific input sequences here
    
    return true;
}

// Get the owner's style component
UStyleComponent* UMeleeWeaponManagerComponent::GetOwnerStyleComponent() const
{
    if (GetOwner())
    {
        return GetOwner()->FindComponentByClass<UStyleComponent>();
    }
    return nullptr;
}

// Get the owner's momentum component
UMomentumComponent* UMeleeWeaponManagerComponent::GetOwnerMomentumComponent() const
{
    if (GetOwner())
    {
        return GetOwner()->FindComponentByClass<UMomentumComponent>();
    }
    return nullptr;
}

// Get the owner's rift component
URiftComponent* UMeleeWeaponManagerComponent::GetOwnerRiftComponent() const
{
    if (GetOwner())
    {
        return GetOwner()->FindComponentByClass<URiftComponent>();
    }
    return nullptr;
}

// Get the owner's weapon manager component
UWeaponManagerComponent* UMeleeWeaponManagerComponent::GetOwnerWeaponManagerComponent() const
{
    if (GetOwner())
    {
        return GetOwner()->FindComponentByClass<UWeaponManagerComponent>();
    }
    return nullptr;
}