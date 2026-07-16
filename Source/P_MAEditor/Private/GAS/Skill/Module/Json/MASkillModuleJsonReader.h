#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/Json/MASkillModuleDataValidator.h"

struct FMASkillModuleJsonSource;

struct FMASkillModuleReadResult
{
	int32 ModuleId = 0;
	FMASkillModuleData ModuleData;
	TArray<FMASkillModuleDiagnostic> Diagnostics;

	bool IsValid() const;
	FText GetDiagnosticsText() const;
};

struct FMASkillModuleJsonReader
{
	static bool ReadHeader(
		const FString& Json,
		int32& OutModuleId,
		FName& OutModuleName,
		FText& OutError);

	static FMASkillModuleReadResult Read(const FString& Json, UObject& AddonOuter);
	static FMASkillModuleReadResult Read(const FMASkillModuleJsonSource& Source, UObject& AddonOuter);
};
