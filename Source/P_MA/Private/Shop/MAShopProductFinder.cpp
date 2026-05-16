#include "Shop/MAShopProductFinder.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "Modules/ModuleManager.h"

void FMAShopProductFinder::FindSkillModules(const TArray<FDirectoryPath>& RootPaths, TArray<UMASkillDefinition*>& OutSkillDefinitions)
{
	TArray<FAssetData> Assets;
	FindAssetsByClass(RootPaths, UMASkillDefinition::StaticClass(), Assets);

	for (const FAssetData& Asset : Assets)
	{
		UMASkillDefinition* SkillDefinition = Cast<UMASkillDefinition>(Asset.GetAsset());
		if (!SkillDefinition) continue;

		OutSkillDefinitions.Add(SkillDefinition);
	}
}

FName FMAShopProductFinder::MakePackagePath(const FDirectoryPath& RootPath)
{
	FString Path = RootPath.Path;
	Path.ReplaceInline(TEXT("\\"), TEXT("/"));

	if (Path.StartsWith(TEXT("Content/")))
	{
		Path = FString(TEXT("/Game/")) + Path.RightChop(8);
	}
	else if (!Path.StartsWith(TEXT("/")))
	{
		Path = FString(TEXT("/Game/")) + Path;
	}

	return Path.IsEmpty() ? NAME_None : FName(*Path);
}

void FMAShopProductFinder::FindAssetsByClass(const TArray<FDirectoryPath>& RootPaths, const UClass* AssetClass, TArray<FAssetData>& OutAssets)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	for (const FDirectoryPath& RootPath : RootPaths)
	{
		const FName PackagePath = MakePackagePath(RootPath);
		if (PackagePath.IsNone()) continue;

		FARFilter Filter;
		Filter.PackagePaths.Add(PackagePath);
		Filter.ClassPaths.Add(AssetClass->GetClassPathName());
		Filter.bRecursivePaths = true;

		AssetRegistry.GetAssets(Filter, OutAssets);
	}
}
