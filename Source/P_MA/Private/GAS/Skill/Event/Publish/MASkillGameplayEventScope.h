#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

class UMASkillRuntimeScope;

namespace MASkillGameplayEventScope
{
	void InjectRuntimeScope(FGameplayEventData& Payload, UMASkillRuntimeScope* InRuntimeScope);
	const UMASkillRuntimeScope* ExtractRuntimeScope(const FGameplayEventData& Payload);
}

