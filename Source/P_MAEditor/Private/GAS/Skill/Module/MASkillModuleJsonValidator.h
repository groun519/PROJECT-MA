#pragma once

#include "CoreMinimal.h"

struct FMASkillModuleData;

struct FMASkillModuleJsonValidator
{
	static bool Validate(int32 ModuleId, const FMASkillModuleData& ModuleData, FText& OutError);
};
