#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/Build/MASkillModuleBuildTypes.h"

struct FMASkillModuleBuildPipeline
{
	static bool CollectStatus(
		const FString& SourceDirectory,
		TArray<FMASkillModuleBuildItem>& OutItems,
		FText& OutError);

	static bool BuildFile(
		const FString& SourceDirectory,
		const FString& JsonFile,
		FMASkillModuleBuildSummary& OutSummary,
		FText& OutError);

	static bool BuildFiles(
		const FString& SourceDirectory,
		const TArray<FString>& JsonFiles,
		EMASkillModuleBuildMode BuildMode,
		FMASkillModuleBuildSummary& OutSummary,
		FText& OutError);

	static bool DeleteGeneratedAsset(
		const FSoftObjectPath& GeneratedAsset,
		FText& OutError);

	static bool ResolveNextModuleId(
		const FString& SourceDirectory,
		int32& OutModuleId,
		FText& OutError);

	static bool ResolveGeneratedAssetDirectory(FString& OutDirectory, FText& OutError);
};
