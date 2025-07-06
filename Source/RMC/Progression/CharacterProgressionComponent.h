// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterProgressionComponent.generated.h"

// Skill data structure
USTRUCT(BlueprintType)
struct FSkillData
{
	GENERATED_BODY()

	// Display name of the skill
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	FText DisplayName;

	// Description of the skill
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	FText Description;

	// Skill point cost to unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	int32 Cost;

	// Character level required to unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	int32 LevelRequirement;

	// Prerequisite skills that must be unlocked first
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	TArray<FName> Prerequisites;

	// Default constructor
	FSkillData()
		: DisplayName(FText::FromString("Skill"))
		, Description(FText::FromString("A skill that can be unlocked"))
		, Cost(1)
		, LevelRequirement(1)
	{
	}

	// Constructor with parameters
	FSkillData(FText InDisplayName, FText InDescription, int32 InCost, TArray<FName> InPrerequisites, int32 InLevelRequirement = 1)
		: DisplayName(InDisplayName)
		, Description(InDescription)
		, Cost(InCost)
		, LevelRequirement(InLevelRequirement)
		, Prerequisites(InPrerequisites)
	{
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RMC_API UCharacterProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterProgressionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Character level and XP
	UPROPERTY(BlueprintReadOnly, Category = "Progression|Level")
	int32 CurrentXP;

	UPROPERTY(BlueprintReadOnly, Category = "Progression|Level")
	int32 CurrentLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Level")
	TArray<int32> LevelXPThresholds;

	// Character rank (overall progression)
	UPROPERTY(BlueprintReadOnly, Category = "Progression|Rank")
	int32 CurrentRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Rank")
	TArray<int32> RankThresholds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Rank")
	TArray<FText> RankNames;

	// Skill points and unlocked skills
	UPROPERTY(BlueprintReadOnly, Category = "Progression|Skills")
	int32 SkillPoints;

	UPROPERTY(BlueprintReadOnly, Category = "Progression|Skills")
	TArray<FName> UnlockedSkills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Skills")
	TMap<FName, FSkillData> SkillTree;

	// Currencies
	UPROPERTY(BlueprintReadOnly, Category = "Progression|Currency")
	int32 StyleOrbs;

	UPROPERTY(BlueprintReadOnly, Category = "Progression|Currency")
	int32 RiftOrbs;

	UPROPERTY(BlueprintReadOnly, Category = "Progression|Currency")
	int32 RaritaniumShards;

	// Rift progression
	UPROPERTY(BlueprintReadOnly, Category = "Progression|Rift")
	int32 RiftEnergy;

	UPROPERTY(BlueprintReadOnly, Category = "Progression|Rift")
	int32 RiftAttunementLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Rift")
	TArray<int32> RiftEnergyThresholds;

	// Style progression
	UPROPERTY(BlueprintReadOnly, Category = "Progression|Style")
	int32 StyleExperience;

	UPROPERTY(BlueprintReadOnly, Category = "Progression|Style")
	int32 StyleMasteryLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression|Style")
	TArray<int32> StyleExperienceThresholds;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddCharacterXP(int XPAmount);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddSkillPoints(int Points);

	UFUNCTION(BlueprintCallable, Category = "Progression|Skills")
	bool UnlockSkill(FName SkillID);

	UFUNCTION(BlueprintPure, Category = "Progression|Skills")
	bool CanUnlockSkill(FName SkillID) const;

	UFUNCTION(BlueprintPure, Category = "Progression|Skills")
	bool HasSkill(FName SkillID) const;

	UFUNCTION(BlueprintCallable, Category = "Progression|Rift")
	void AddRiftEnergy(int Energy);

	UFUNCTION(BlueprintCallable, Category = "Progression|Rift")
	void UpdateRiftCapabilities();

	UFUNCTION(BlueprintPure, Category = "Progression|Rift")
	int GetRiftEnergyToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Progression|Rift")
	float GetRiftAttunementProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Progression|Style")
	void AddStyleExperience(int Experience);

	UFUNCTION(BlueprintCallable, Category = "Progression|Style")
	void UpdateStyleCapabilities();

	UFUNCTION(BlueprintPure, Category = "Progression|Style")
	int GetStyleExperienceToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Progression|Style")
	float GetStyleMasteryProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Progression|Currency")
	void AddStyleOrbs(int Orbs);

	UFUNCTION(BlueprintCallable, Category = "Progression|Currency")
	void AddRiftOrbs(int Orbs);

	UFUNCTION(BlueprintCallable, Category = "Progression|Currency")
	void AddRaritaniumShards(int Shards);

	UFUNCTION(BlueprintCallable, Category = "Progression|Currency")
	bool SpendStyleOrbs(int Orbs);

	UFUNCTION(BlueprintCallable, Category = "Progression|Currency")
	bool SpendRiftOrbs(int Orbs);

	UFUNCTION(BlueprintCallable, Category = "Progression|Currency")
	bool SpendRaritaniumShards(int Shards);

	UFUNCTION(BlueprintPure, Category = "Progression|Rank")
	FText GetCurrentRankDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Progression|Rank")
	int GetXPToNextRank() const;

	UFUNCTION(BlueprintPure, Category = "Progression|Rank")
	float GetRankProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Progression|Save")
	void SaveProgression();

	UFUNCTION(BlueprintCallable, Category = "Progression|Save")
	void LoadProgression();

	// Events
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnLevelUp OnLevelUp;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRankUp, int32, NewRank);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnRankUp OnRankUp;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPGained, int32, XPGained, int32, TotalXP);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnXPGained OnXPGained;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillPointsChanged, int32, NewSkillPoints);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnSkillPointsChanged OnSkillPointsChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillUnlocked, FName, SkillID);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnSkillUnlocked OnSkillUnlocked;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrencyChanged);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnCurrencyChanged OnCurrencyChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRiftAttunementLevelUp, int32, NewLevel);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnRiftAttunementLevelUp OnRiftAttunementLevelUp;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRiftEnergyGained, int32, EnergyGained, int32, TotalEnergy);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnRiftEnergyGained OnRiftEnergyGained;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRiftCapabilitiesUpdated, int32, AttunementLevel);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnRiftCapabilitiesUpdated OnRiftCapabilitiesUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStyleMasteryLevelUp, int32, NewLevel);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnStyleMasteryLevelUp OnStyleMasteryLevelUp;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStyleExperienceGained, int32, ExperienceGained, int32, TotalExperience);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnStyleExperienceGained OnStyleExperienceGained;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStyleCapabilitiesUpdated, int32, MasteryLevel);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnStyleCapabilitiesUpdated OnStyleCapabilitiesUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProgressionSaved);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnProgressionSaved OnProgressionSaved;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProgressionLoaded);
	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnProgressionLoaded OnProgressionLoaded;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};