#include "GAS/Skill/Event/Publish/MASkillGameplayEventScope.h"

#include "GAS/Skill/Module/MASkillModuleInstance.h"

void MASkillGameplayEventScope::InjectRuntimeScope(FGameplayEventData& EventData, UMASkillModuleInstance* InRuntimeScope)
{
	EventData.OptionalObject = InRuntimeScope;
}

UMASkillModuleInstance* MASkillGameplayEventScope::ExtractRuntimeScope(FGameplayEventData& EventData)
{
	return const_cast<UMASkillModuleInstance*>(Cast<UMASkillModuleInstance>(EventData.OptionalObject));
}

