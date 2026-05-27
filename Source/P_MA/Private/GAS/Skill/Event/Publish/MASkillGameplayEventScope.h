#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

class UMASkillModuleInstance;

class MASkillGameplayEventScope final
{
public:
	static void InjectRuntimeScope(FGameplayEventData& EventData, UMASkillModuleInstance* InRuntimeScope);
	static UMASkillModuleInstance* ExtractRuntimeScope(FGameplayEventData& EventData);

private:
	MASkillGameplayEventScope() = delete;
};
