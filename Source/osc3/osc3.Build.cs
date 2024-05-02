using System;
using System.IO;
using UnrealBuildTool;

public class osc3 : ModuleRules
{
	public osc3(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
    PublicIncludePaths.AddRange(new string[] { "osc3" });
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OSC", "UMG", "DefinitivePainter",  "HeadMountedDisplay", "EnhancedInput", "CableComponent", "Niagara" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG", "DefinitivePainter", "Json", "JsonUtilities" });

    PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "lib", "svgrender", "svgrender.lib"));
	}
}
