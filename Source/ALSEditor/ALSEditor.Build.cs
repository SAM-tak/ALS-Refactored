using UnrealBuildTool;

public class ALSEditor : ModuleRules
{
	public ALSEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		CppCompileWarningSettings.NonInlinedGenCppWarningLevel = WarningLevel.Warning;

		PublicDependencyModuleNames.AddRange(
		[
			"Core", "CoreUObject", "Engine", "AnimationModifiers", "AnimationBlueprintLibrary", "ALS"
		]);

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(
			[
				"AnimGraph"
			]);

			PrivateDependencyModuleNames.AddRange(
			[
				"BlueprintGraph", "Slate", "SlateCore", "Projects"
			]);
		}
	}
}