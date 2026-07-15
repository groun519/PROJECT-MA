using System.IO;
using UnrealBuildTool;

public class P_MAEditor : ModuleRules
{
	public P_MAEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"AIModule",
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayAbilities",
			"Json",
			"JsonUtilities",
			"P_MA",
			"StructUtils"
		});

		// PROJECT-MA currently keeps runtime headers under P_MA/Private.
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../P_MA/Private"));
	}
}
