// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RackSimTarget : TargetRules
{
  public RackSimTarget(TargetInfo Target) : base(Target)
  {
    Type = TargetType.Game;
    DefaultBuildSettings = BuildSettingsVersion.V2;
    IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
    ExtraModuleNames.Add("RackSim");

    bUseLoggingInShipping = true;
  }
}