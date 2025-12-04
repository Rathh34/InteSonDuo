using UnrealBuildTool;

public class TensionAudioManager : ModuleRules
{
	public TensionAudioManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
			"AudioMixer",
			"Slate",
			"SlateCore"
		});
	}
}