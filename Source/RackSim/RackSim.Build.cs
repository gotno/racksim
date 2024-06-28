using System;
using System.IO;
using UnrealBuildTool;

public class RackSim : ModuleRules
{
	public RackSim(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
    PublicIncludePaths.AddRange(new string[] { "RackSim" });
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OSC", "UMG", "HeadMountedDisplay", "EnhancedInput", "CableComponent", "Niagara" });
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG", "Json", "JsonUtilities" });

		PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty", "lib", "svgrender", "svgrender.lib"));
	}
}
