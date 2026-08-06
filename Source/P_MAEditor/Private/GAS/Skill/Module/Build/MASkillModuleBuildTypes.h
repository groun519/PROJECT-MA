#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MASkillModuleTypes.h"

enum class EMASkillModuleBuildStatus : uint8
{
	Error,
	NeedsBuild,
	Built
};

enum class EMASkillModuleBuildMode : uint8
{
	IfRequired,
	Force
};

struct FMASkillModuleBuildItem
{
	int32 ModuleId = 0;
	EMASkillModuleType ModuleType = EMASkillModuleType::None;
	FString SourceFile;
	FSoftObjectPath GeneratedAssetPath;
	EMASkillModuleBuildStatus Status = EMASkillModuleBuildStatus::Error;
	int64 LastBuiltAt = 0;
	FText StatusDetail;

	bool CanBuild() const
	{
		return !SourceFile.IsEmpty() && Status != EMASkillModuleBuildStatus::Error;
	}

	bool RequiresBuild() const
	{
		return Status == EMASkillModuleBuildStatus::NeedsBuild;
	}
};

struct FMASkillModuleBuildFailure
{
	FString SourceFile;
	FText Error;
};

struct FMASkillModuleBuildSummary
{
	int32 Built = 0;
	int32 UpToDate = 0;
	TArray<FMASkillModuleBuildFailure> Failures;
};
