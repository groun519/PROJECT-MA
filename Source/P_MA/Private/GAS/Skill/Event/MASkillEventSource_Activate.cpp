#include "GAS/Skill/Event/MASkillEventSource_Activate.h"

void UMASkillEventSource_Activate::StartSource(UMASkillAbility* SkillAbility)
{
	Super::StartSource(SkillAbility);
	EmitEvent();
}
