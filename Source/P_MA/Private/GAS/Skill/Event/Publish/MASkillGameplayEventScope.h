#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

class UMASkillModuleInstance;

class MASkillGameplayEventScope final
{
public:
	static void InjectBindingScope(FGameplayEventData& EventData, UMASkillModuleInstance* InBindingScope);
	static UMASkillModuleInstance* ExtractBindingScope(FGameplayEventData& EventData);

private:
	MASkillGameplayEventScope() = delete;
};
