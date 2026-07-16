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
			"AssetRegistry",
			"Core",
			"CoreUObject",
			"DesktopPlatform",
			"Engine",
			"GameplayAbilities",
			"InputCore",
			"Json",
			"JsonUtilities",
			"P_MA",
			"PropertyEditor",
			"Slate",
			"SlateCore",
			"StructUtils",
			"UnrealEd",
			"WorkspaceMenuStructure"
		});

		// PROJECT-MA currently keeps runtime headers under P_MA/Private.
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../P_MA/Private"));
	}
}
