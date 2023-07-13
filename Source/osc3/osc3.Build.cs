// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class osc3 : ModuleRules
{
	public osc3(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OSC", "UMG", "DefinitivePainter" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG", "DefinitivePainter" });
	}
}
