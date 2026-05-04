#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

class UMASkillRuntimeScope;

class MASkillGameplayEventScope final
{
public:
	static void InjectRuntimeScope(FGameplayEventData& Payload, UMASkillRuntimeScope* InRuntimeScope);
	static const UMASkillRuntimeScope* ExtractRuntimeScope(const FGameplayEventData& Payload);

private:
	MASkillGameplayEventScope() = delete;
};
