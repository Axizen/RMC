﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RMC : ModuleRules
{
	public RMC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {     
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "EnhancedInput",
    "PhysicsCore",
    "NavigationSystem",
    "GeometryCollectionEngine",
    "FieldSystemEngine",
    "ChaosSolverEngine",
    "Niagara" });
		
		// Include directories
		PrivateIncludePaths.AddRange(new string[] {
			"RMC/Core",
			"RMC/Combat",
			"RMC/Weapons",
			"RMC/Enemies"
		});
		
		// Enable more warnings and treat them as errors
		PublicDefinitions.Add("WITH_DOUBLED_JUMP=1");
	}
}