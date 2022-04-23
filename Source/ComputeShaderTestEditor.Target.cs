// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ComputeShaderTestEditorTarget : TargetRules
{
	public ComputeShaderTestEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "ComputeShaderTest"} );
		ExtraModuleNames.Add("ComputeShaders");
	}
}
