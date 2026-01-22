using UnrealBuildTool;

public class GameAudioDirector : ModuleRules
{
	public GameAudioDirector(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
			"AudioMixer",
		});
	}
}