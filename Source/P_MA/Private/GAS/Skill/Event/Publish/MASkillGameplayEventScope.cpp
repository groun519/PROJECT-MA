#include "GAS/Skill/Event/Publish/MASkillGameplayEventScope.h"

#include "GAS/Skill/Module/MASkillModuleInstance.h"

void MASkillGameplayEventScope::InjectBindingScope(FGameplayEventData& EventData, UMASkillModuleInstance* InBindingScope)
{
	EventData.OptionalObject = InBindingScope;
}

UMASkillModuleInstance* MASkillGameplayEventScope::ExtractBindingScope(FGameplayEventData& EventData)
{
	return const_cast<UMASkillModuleInstance*>(Cast<UMASkillModuleInstance>(EventData.OptionalObject));
}

