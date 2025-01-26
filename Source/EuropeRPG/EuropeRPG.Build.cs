// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EuropeRPG : ModuleRules
{
	public EuropeRPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "AIModule","CustomComputeShader", "ImageWriteQueue" });
    }
}
