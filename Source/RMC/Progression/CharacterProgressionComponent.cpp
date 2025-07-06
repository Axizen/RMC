// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterProgressionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"

// Sets default values for this component's properties
UCharacterProgressionComponent::UCharacterProgressionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	CurrentXP = 0;
	CurrentLevel = 1;
	CurrentRank = 0;
	SkillPoints = 0;
	StyleOrbs = 0;
	RiftOrbs = 0;
	RaritaniumShards = 0;
	RiftEnergy = 0;
	RiftAttunementLevel = 1;
	StyleExperience = 0;
	StyleMasteryLevel = 1;

	// Default XP thresholds for levels
	LevelXPThresholds = { 1000, 2500, 5000, 10000, 20000 };

	// Default rank thresholds
	RankThresholds = { 0, 5000, 15000, 30000, 50000, 75000, 100000 };

	// Default rank names
	RankNames.Add(FText::FromString("Novice"));
	RankNames.Add(FText::FromString("Adept"));
	RankNames.Add(FText::FromString("Expert"));
	RankNames.Add(FText::FromString("Master"));
	RankNames.Add(FText::FromString("Grandmaster"));
	RankNames.Add(FText::FromString("Legend"));
	RankNames.Add(FText::FromString("Mythic"));

	// Default rift energy thresholds
	RiftEnergyThresholds = { 1000, 2500, 5000, 10000, 20000 };

	// Default style experience thresholds
	StyleExperienceThresholds = { 1000, 2500, 5000, 10000, 20000 };

	// Default skill tree
	// Basic movement skills
	SkillTree.Add(FName("DoubleJump"), FSkillData(FText::FromString("Double Jump"), FText::FromString("Allows a second jump while in the air"), 1, TArray<FName>()));
	SkillTree.Add(FName("AirDash"), FSkillData(FText::FromString("Air Dash"), FText::FromString("Dash quickly through the air"), 2, { FName("DoubleJump") }));
	SkillTree.Add(FName("WallRun"), FSkillData(FText::FromString("Wall Run"), FText::FromString("Run along walls for a short time"), 2, { FName("DoubleJump") }));

	// Rift skills
	SkillTree.Add(FName("RiftChain"), FSkillData(FText::FromString("Rift Chain"), FText::FromString("Chain multiple rifts together"), 3, { FName("AirDash") }));
	SkillTree.Add(FName("RiftSurge"), FSkillData(FText::FromString("Rift Surge"), FText::FromString("Gain a burst of speed after rifting"), 2, { FName("RiftChain") }));
	SkillTree.Add(FName("RiftCounter"), FSkillData(FText::FromString("Rift Counter"), FText::FromString("Counter enemy attacks with a rift"), 4, { FName("RiftSurge") }));

	// Combat skills
	SkillTree.Add(FName("AerialRecovery"), FSkillData(FText::FromString("Aerial Recovery"), FText::FromString("Recover quickly when knocked into the air"), 2, { FName("DoubleJump") }));
	SkillTree.Add(FName("StyleBoost"), FSkillData(FText::FromString("Style Boost"), FText::FromString("Gain more style points from actions"), 3, { FName("AerialRecovery") }));
	SkillTree.Add(FName("MomentumMastery"), FSkillData(FText::FromString("Momentum Mastery"), FText::FromString("Momentum decays slower"), 4, { FName("StyleBoost") }));
}

// Called when the game starts
void UCharacterProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Load saved progression data
	LoadProgression();

	// Update capabilities based on current progression
	UpdateRiftCapabilities();
	UpdateStyleCapabilities();
}

// Called every frame
void UCharacterProgressionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCharacterProgressionComponent::AddCharacterXP(int XPAmount)
{
	// Add XP
	CurrentXP += XPAmount;

	// Check for level up
	while (CurrentLevel <= LevelXPThresholds.Num() && 
		   CurrentXP >= LevelXPThresholds[CurrentLevel - 1])
	{
		// Level up
		CurrentLevel++;
		
		// Award skill points
		SkillPoints += 2;
		
		// Broadcast level up event
		OnLevelUp.Broadcast(CurrentLevel);
	}

	// Check for rank up
	int NewRank = 0;
	for (int i = RankThresholds.Num() - 1; i >= 0; --i)
	{
		if (CurrentXP >= RankThresholds[i])
		{
			NewRank = i;
			break;
		}
	}

	if (NewRank != CurrentRank)
	{
		CurrentRank = NewRank;
		OnRankUp.Broadcast(CurrentRank);
	}

	// Broadcast XP gained event
	OnXPGained.Broadcast(XPAmount, CurrentXP);

	// Save progression
	SaveProgression();
}

void UCharacterProgressionComponent::AddSkillPoints(int Points)
{
	SkillPoints += Points;
	OnSkillPointsChanged.Broadcast(SkillPoints);
	SaveProgression();
}

bool UCharacterProgressionComponent::UnlockSkill(FName SkillID)
{
	// Check if skill exists and can be unlocked
	if (!CanUnlockSkill(SkillID))
	{
		return false;
	}

	// Get skill data
	FSkillData* SkillData = SkillTree.Find(SkillID);
	if (!SkillData)
	{
		return false;
	}

	// Check if we have enough skill points
	if (SkillPoints < SkillData->Cost)
	{
		return false;
	}

	// Spend skill points
	SkillPoints -= SkillData->Cost;

	// Add to unlocked skills
	UnlockedSkills.Add(SkillID);

	// Broadcast events
	OnSkillUnlocked.Broadcast(SkillID);
	OnSkillPointsChanged.Broadcast(SkillPoints);

	// Save progression
	SaveProgression();

	return true;
}

bool UCharacterProgressionComponent::CanUnlockSkill(FName SkillID) const
{
	// Check if skill exists
	const FSkillData* SkillData = SkillTree.Find(SkillID);
	if (!SkillData)
	{
		return false;
	}

	// Check if already unlocked
	if (UnlockedSkills.Contains(SkillID))
	{
		return false;
	}

	// Check if we have enough skill points
	if (SkillPoints < SkillData->Cost)
	{
		return false;
	}

	// Check if we meet level requirements
	if (CurrentLevel < SkillData->LevelRequirement)
	{
		return false;
	}

	// Check if we have all prerequisites
	for (FName Prerequisite : SkillData->Prerequisites)
	{
		if (!UnlockedSkills.Contains(Prerequisite))
		{
			return false;
		}
	}

	return true;
}

bool UCharacterProgressionComponent::HasSkill(FName SkillID) const
{
	return UnlockedSkills.Contains(SkillID);
}

void UCharacterProgressionComponent::AddRiftEnergy(int Energy)
{
	// Add energy
	RiftEnergy += Energy;

	// Check for attunement level up
	while (RiftAttunementLevel <= RiftEnergyThresholds.Num() && 
		   RiftEnergy >= RiftEnergyThresholds[RiftAttunementLevel - 1])
	{
		// Level up
		RiftAttunementLevel++;
		
		// Broadcast level up event
		OnRiftAttunementLevelUp.Broadcast(RiftAttunementLevel);
		
		// Update rift capabilities
		UpdateRiftCapabilities();
	}

	// Broadcast energy gained event
	OnRiftEnergyGained.Broadcast(Energy, RiftEnergy);

	// Save progression
	SaveProgression();
}

void UCharacterProgressionComponent::UpdateRiftCapabilities()
{
	// This would update the RiftComponent with new capabilities based on attunement level
	// For now, just broadcast an event that other components can listen to
	OnRiftCapabilitiesUpdated.Broadcast(RiftAttunementLevel);
}

int UCharacterProgressionComponent::GetRiftEnergyToNextLevel() const
{
	// If we're at max level, return 0
	if (RiftAttunementLevel > RiftEnergyThresholds.Num())
	{
		return 0;
	}

	// Return energy needed for next level
	return RiftEnergyThresholds[RiftAttunementLevel - 1] - RiftEnergy;
}

float UCharacterProgressionComponent::GetRiftAttunementProgress() const
{
	// If we're at max level, return 1.0
	if (RiftAttunementLevel > RiftEnergyThresholds.Num())
	{
		return 1.0f;
	}

	// Calculate progress as percentage to next level
	float CurrentLevelEnergy = (RiftAttunementLevel > 1) ? RiftEnergyThresholds[RiftAttunementLevel - 2] : 0.0f;
	float NextLevelEnergy = RiftEnergyThresholds[RiftAttunementLevel - 1];
	float LevelEnergyRange = NextLevelEnergy - CurrentLevelEnergy;

	return (RiftEnergy - CurrentLevelEnergy) / LevelEnergyRange;
}

void UCharacterProgressionComponent::AddStyleExperience(int Experience)
{
	// Add experience
	StyleExperience += Experience;

	// Check for mastery level up
	while (StyleMasteryLevel <= StyleExperienceThresholds.Num() && 
		   StyleExperience >= StyleExperienceThresholds[StyleMasteryLevel - 1])
	{
		// Level up
		StyleMasteryLevel++;
		
		// Broadcast level up event
		OnStyleMasteryLevelUp.Broadcast(StyleMasteryLevel);
		
		// Update style capabilities
		UpdateStyleCapabilities();
	}

	// Broadcast experience gained event
	OnStyleExperienceGained.Broadcast(Experience, StyleExperience);

	// Save progression
	SaveProgression();
}

void UCharacterProgressionComponent::UpdateStyleCapabilities()
{
	// This would update the StyleComponent with new capabilities based on mastery level
	// For now, just broadcast an event that other components can listen to
	OnStyleCapabilitiesUpdated.Broadcast(StyleMasteryLevel);
}

int UCharacterProgressionComponent::GetStyleExperienceToNextLevel() const
{
	// If we're at max level, return 0
	if (StyleMasteryLevel > StyleExperienceThresholds.Num())
	{
		return 0;
	}

	// Return experience needed for next level
	return StyleExperienceThresholds[StyleMasteryLevel - 1] - StyleExperience;
}

float UCharacterProgressionComponent::GetStyleMasteryProgress() const
{
	// If we're at max level, return 1.0
	if (StyleMasteryLevel > StyleExperienceThresholds.Num())
	{
		return 1.0f;
	}

	// Calculate progress as percentage to next level
	float CurrentLevelExp = (StyleMasteryLevel > 1) ? StyleExperienceThresholds[StyleMasteryLevel - 2] : 0.0f;
	float NextLevelExp = StyleExperienceThresholds[StyleMasteryLevel - 1];
	float LevelExpRange = NextLevelExp - CurrentLevelExp;

	return (StyleExperience - CurrentLevelExp) / LevelExpRange;
}

void UCharacterProgressionComponent::AddStyleOrbs(int Orbs)
{
	StyleOrbs += Orbs;
	OnCurrencyChanged.Broadcast();
	SaveProgression();
}

void UCharacterProgressionComponent::AddRiftOrbs(int Orbs)
{
	RiftOrbs += Orbs;
	OnCurrencyChanged.Broadcast();
	SaveProgression();
}

void UCharacterProgressionComponent::AddRaritaniumShards(int Shards)
{
	RaritaniumShards += Shards;
	OnCurrencyChanged.Broadcast();
	SaveProgression();
}

bool UCharacterProgressionComponent::SpendStyleOrbs(int Orbs)
{
	if (StyleOrbs >= Orbs)
	{
		StyleOrbs -= Orbs;
		OnCurrencyChanged.Broadcast();
		SaveProgression();
		return true;
	}
	return false;
}

bool UCharacterProgressionComponent::SpendRiftOrbs(int Orbs)
{
	if (RiftOrbs >= Orbs)
	{
		RiftOrbs -= Orbs;
		OnCurrencyChanged.Broadcast();
		SaveProgression();
		return true;
	}
	return false;
}

bool UCharacterProgressionComponent::SpendRaritaniumShards(int Shards)
{
	if (RaritaniumShards >= Shards)
	{
		RaritaniumShards -= Shards;
		OnCurrencyChanged.Broadcast();
		SaveProgression();
		return true;
	}
	return false;
}

FText UCharacterProgressionComponent::GetCurrentRankDisplayName() const
{
	if (CurrentRank < RankNames.Num())
	{
		return RankNames[CurrentRank];
	}
	return FText::FromString("Unknown Rank");
}

int UCharacterProgressionComponent::GetXPToNextRank() const
{
	// If we're at max rank, return 0
	if (CurrentRank >= RankThresholds.Num() - 1)
	{
		return 0;
	}

	// Return XP needed for next rank
	return RankThresholds[CurrentRank + 1] - CurrentXP;
}

float UCharacterProgressionComponent::GetRankProgress() const
{
	// If we're at max rank, return 1.0
	if (CurrentRank >= RankThresholds.Num() - 1)
	{
		return 1.0f;
	}

	// Calculate progress as percentage to next rank
	float CurrentRankXP = RankThresholds[CurrentRank];
	float NextRankXP = RankThresholds[CurrentRank + 1];
	float RankXPRange = NextRankXP - CurrentRankXP;

	return (CurrentXP - CurrentRankXP) / RankXPRange;
}

void UCharacterProgressionComponent::SaveProgression()
{
	// This would save the progression data to a save game
	// For now, just broadcast an event
	OnProgressionSaved.Broadcast();
}

void UCharacterProgressionComponent::LoadProgression()
{
	// This would load the progression data from a save game
	// For now, just broadcast an event
	OnProgressionLoaded.Broadcast();
}