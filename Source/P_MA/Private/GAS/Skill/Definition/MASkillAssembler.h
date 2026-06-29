#pragma once

#include "CoreMinimal.h"

class UMASkillDefinition;
class UMASkillModuleInstance;
struct FGameplayTag;

struct FMASkillAssembler
{
	static UMASkillModuleInstance* Assemble(
		UObject* Outer,
		const FGameplayTag& SlotTag,
		const TArray<TObjectPtr<UMASkillModuleInstance>>& OrderedModuleInstances);
};
