// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class NBodySimulation : ModuleRules
{
	public NBodySimulation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableExceptions = true;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"NBodySimulation"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"Json", 
			"JsonUtilities",
			"EnhancedInput"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"NBodySim", 
			"RHI",
			"HTTP"	
		});
	}
}
