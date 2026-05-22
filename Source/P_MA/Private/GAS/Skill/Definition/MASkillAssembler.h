#pragma once

#include "CoreMinimal.h"

class UMASkillDefinition;
class UMASkillModuleInstance;

struct FMASkillAssembler
{
	static UMASkillDefinition* Assemble(UObject* Outer, const TArray<TObjectPtr<UMASkillModuleInstance>>& OrderedModuleInstances);
};
