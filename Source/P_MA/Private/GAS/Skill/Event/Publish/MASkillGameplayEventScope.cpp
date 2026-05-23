#include "GAS/Skill/Event/Publish/MASkillGameplayEventScope.h"

#include "GAS/Skill/Module/MASkillModuleInstance.h"

void MASkillGameplayEventScope::InjectRuntimeScope(FGameplayEventData& Payload, UMASkillModuleInstance* InRuntimeScope)
{
	Payload.OptionalObject = InRuntimeScope;
}

const UMASkillModuleInstance* MASkillGameplayEventScope::ExtractRuntimeScope(const FGameplayEventData& Payload)
{
	return Cast<UMASkillModuleInstance>(Payload.OptionalObject);
}

