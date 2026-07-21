#pragma once

#include "CoreMinimal.h"

enum class EMAModuleRarity : uint8;

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
		int32& OutModuleId,
		FName& OutModuleName,
		EMAModuleRarity& OutModuleRarity,
		FText& OutError);

	static bool ResolveModuleId(const FString& SourceFile, int32& OutModuleId, FText& OutError);

	static bool ResolveModuleIdFromContent(
		const FString& SourceFile,
		int32& OutModuleId,
		FText& OutError);

};
