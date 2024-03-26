using UnrealBuildTool;

public class ALS : ModuleRules
{
	public ALS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		bEnableNonInlinedGenCppWarnings = true;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "EnhancedInput", "GameplayTags", "GameplayAbilities", "GameplayTasks", "ModularGameplay"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"EngineSettings", "NetCore", "PhysicsCore", "Niagara", "AnimGraphRuntime", "RigVM", "ControlRig", "Chaos", "MotionWarping"
		});

		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new[] {"MessageLog"});
		}
	}
}