#include "GAS/Skill/Event/Publish/MASkillEventSource_Activate.h"

void UMASkillEventSource_Activate::StartSource(UMASkillAbility* SkillAbility)
{
	Super::StartSource(SkillAbility);
	EmitEvent();
}
