#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"

struct FMASkillModuleJsonHeader
{
	int32 ModuleId = 0;
	FName ModuleName = NAME_None;
	EMAModuleRarity ModuleRarity = EMAModuleRarity::Rarity4;
	EMASkillModuleType ModuleType = EMASkillModuleType::Module;
};

struct FMASkillModuleJsonSource
{
	int32 ModuleId = 0;
	TArray<uint8> Bytes;
	FString SourceHash;

	FString ToJson() const;
};

/** Shared file operations for module JSON sources. */
struct FMASkillModuleJsonFile
{
	static bool Load(
		const FString& FilePath,
		FMASkillModuleJsonSource& OutSource,
		FText& OutError);

	static bool Save(
		const FString& FilePath,
		const FString& Json,
		bool bReplaceExisting,
		FText& OutError);

	static bool ReadHeader(
		const FString& FilePath,
		FMASkillModuleJsonHeader& OutHeader,
		FText& OutError);

	static FString MakeSourceFilePath(
		const FString& SourceDirectory,
		int32 ModuleId,
		EMASkillModuleType ModuleType);
	static bool ValidateSourceFilePath(
		const FString& SourceDirectory,
		const FString& SourceFile,
		EMASkillModuleType ModuleType,
		FText& OutError);

	static bool ResolveModuleId(const FString& SourceFile, int32& OutModuleId, FText& OutError);

	static bool ResolveModuleIdFromContent(
		const FString& SourceFile,
		int32& OutModuleId,
		FText& OutError);

};
