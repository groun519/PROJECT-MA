#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

class UMASkillModuleInstance;

class MASkillGameplayEventScope final
{
public:
	static void InjectRuntimeScope(FGameplayEventData& Payload, UMASkillModuleInstance* InRuntimeScope);
	static const UMASkillModuleInstance* ExtractRuntimeScope(const FGameplayEventData& Payload);

private:
	MASkillGameplayEventScope() = delete;
};
