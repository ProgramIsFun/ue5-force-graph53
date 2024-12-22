// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class NBodySimulationTarget : TargetRules
{
	public NBodySimulationTarget(TargetInfo Target) : base(Target)
	{


		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("NBodySimulation");
	
	}
}
