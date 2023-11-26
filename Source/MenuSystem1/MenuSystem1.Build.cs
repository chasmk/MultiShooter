// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MenuSystem1 : ModuleRules
{
	public MenuSystem1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Niagara", "MultiplayerSessions",  "HeadMountedDisplay", "OnlineSubsystemSteam", "OnlineSubsystem"});
	}
}
