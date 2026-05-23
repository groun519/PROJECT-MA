#include "GAS/Skill/Event/Publish/MASkillGameplayEventScope.h"

#include "GAS/Skill/Module/MASkillModuleInstance.h"

void MASkillGameplayEventScope::InjectRuntimeScope(FGameplayEventData& Payload, UMASkillModuleInstance* InRuntimeScope)
{
	Payload.OptionalObject = InRuntimeScope;
}

UMASkillModuleInstance* MASkillGameplayEventScope::ExtractRuntimeScope(FGameplayEventData& Payload)
{
	return const_cast<UMASkillModuleInstance*>(Cast<UMASkillModuleInstance>(Payload.OptionalObject));
}

