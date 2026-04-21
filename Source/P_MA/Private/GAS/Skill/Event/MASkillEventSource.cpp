#include "GAS/Skill/Event/MASkillEventSource.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillEventSource::EmitEvent() const
{
	UMASkillDefinition* SkillDefinition = GetTypedOuter<UMASkillDefinition>();
	if (!OwnerSkillAbility || !SkillDefinition || !EmittedTag.IsValid()) return;

	FGameplayEventData Payload;
	Payload.EventTag = EmittedTag;
	SkillDefinition->HandleSkillGameplayEvent(Payload);
}
