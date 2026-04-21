#include "GAS/Skill/Action/MASkillAction_ClearIgnoredActors.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"

void UMASkillAction_ClearIgnoredActors::Execute(UMASkillAbility&, const FGameplayEventData&)
{
	if (UMASkillDefinition* SkillDefinition = GetTypedOuter<UMASkillDefinition>())
	{
		SkillDefinition->ResetActionRuntimeStates();
	}
}
