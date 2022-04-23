using UnrealBuildTool;

public class ComputeShaders : ModuleRules
{
	public ComputeShaders(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			// All the modules we want as string
			"Core",
			"CoreUObject",
			"Engine",
			"Renderer",
			"RenderCore",
			"RHI",
			"Projects"
		});
	}
}