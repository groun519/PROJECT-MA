#include "GAS/Skill/Event/Publish/MASkillEventSource_MontageStart.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillEventSource_MontageStart::HandleSourceEvent(
	UMASkillAbility& SkillAbility,
	UMASkillModuleInstance& InEventScope,
	const FGameplayTag& SourceEventTag,
	const FGameplayEventData& EventData) const
{
	if (SourceEventTag != EmittedTag) return;

	EmitEvent(SkillAbility, InEventScope, EventData);
}
