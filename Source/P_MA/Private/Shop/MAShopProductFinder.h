#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "Shop/MAShopTypes.h"

class UMASkillDefinition;

class FMAShopProductFinder
{
public:
	static void FindSkillModules(const TArray<FDirectoryPath>& RootPaths, TArray<UMASkillDefinition*>& OutSkillDefinitions);

private:
	static FName MakePackagePath(const FDirectoryPath& RootPath);
	static void FindAssetsByClass(const TArray<FDirectoryPath>& RootPaths, const UClass* AssetClass, TArray<FAssetData>& OutAssets);
};
