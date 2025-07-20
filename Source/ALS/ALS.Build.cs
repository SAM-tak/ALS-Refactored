using UnrealBuildTool;

public class ALS : ModuleRules
{
    public ALS(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Warning;

        PublicDependencyModuleNames.AddRange(
		[
			"Core", "CoreUObject", "Engine", "EnhancedInput", "GameplayTags", "GameplayAbilities", "GameplayTasks", "ModularGameplay"
		]);

        PrivateDependencyModuleNames.AddRange(
		[
			"EngineSettings", "NetCore", "PhysicsCore", "Niagara", "AnimGraphRuntime", "RigVM", "ControlRig", "Chaos", "MotionWarping"
		]);

        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(["MessageLog"]);
        }
    }
}