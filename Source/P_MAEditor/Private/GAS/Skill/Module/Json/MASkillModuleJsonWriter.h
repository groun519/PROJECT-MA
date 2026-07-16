#pragma once

#include "CoreMinimal.h"

struct FMASkillModuleData;

struct FMASkillModuleJsonWriter
{
	static bool Write(
		int32 ModuleId,
		const FMASkillModuleData& ModuleData,
		FString& OutJson,
		FText& OutError);
};
