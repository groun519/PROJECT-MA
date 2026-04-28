#pragma once

#include "CoreMinimal.h"

class UMASkillDefinition;

struct FMASkillAssembler
{
	static UMASkillDefinition* Assemble(UObject* Outer, const TArray<TObjectPtr<UMASkillDefinition>>& OrderedDefinitions);
};
