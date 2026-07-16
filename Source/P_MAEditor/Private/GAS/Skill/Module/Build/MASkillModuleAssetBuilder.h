#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/Build/MASkillModuleBuildTypes.h"

enum class EMASkillModuleAssetBuildResult : uint8
{
	Built,
	UpToDate
};

/** Builds one generated module asset from one JSON source file. */
struct FMASkillModuleAssetBuilder
{
	static bool Build(
		const FString& JsonFile,
		const FString& GeneratedAssetDirectory,
		EMASkillModuleBuildMode BuildMode,
		EMASkillModuleAssetBuildResult& OutResult,
		FText& OutError);

	static bool ResolveBuildStatus(
		const FString& JsonFile,
		const FString& GeneratedAssetDirectory,
		FMASkillModuleBuildItem& OutItem,
		FText& OutError);

	static FString MakeAssetPackageName(const FString& GeneratedAssetDirectory, int32 ModuleId);
};
