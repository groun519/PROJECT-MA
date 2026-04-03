#include "GAS/Skill/Event/MASkillEventSource_Activate.h"

void UMASkillEventSource_Activate::StartSource(UMASkillAbility* SkillAbility, FSkillRuntimeContext* InRuntimeContext)
{
	Super::StartSource(SkillAbility, InRuntimeContext);
	EmitEvent();
}
