#pragma once

#include "CoreMinimal.h"

struct FMASkillModuleData;

struct FMASkillModuleJsonReader
{
	static bool Read(
		const FString& Json,
		UObject& AddonOuter,
		int32& OutModuleId,
		FMASkillModuleData& OutModuleData,
		FText& OutError);
};
