#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

class UMASkillModuleInstance;

class MASkillGameplayEventScope final
{
public:
	static void InjectRuntimeScope(FGameplayEventData& Payload, UMASkillModuleInstance* InRuntimeScope);
	static UMASkillModuleInstance* ExtractRuntimeScope(FGameplayEventData& Payload);

private:
	MASkillGameplayEventScope() = delete;
};
