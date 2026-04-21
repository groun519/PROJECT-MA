#include "GAS/Skill/Event/MASkillEventSource.h"

#include "GAS/Skill/MASkillAbility.h"

void UMASkillEventSource::EmitEvent() const
{
	if (OwnerSkillAbility) OwnerSkillAbility->HandleSkillTagEvent(EmittedTag);
}
