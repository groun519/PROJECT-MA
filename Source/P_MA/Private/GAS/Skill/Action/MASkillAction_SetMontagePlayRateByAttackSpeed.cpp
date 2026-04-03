#include "GAS/Skill/Action/MASkillAction_SetMontagePlayRateByAttackSpeed.h"

#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_SetMontagePlayRateByAttackSpeed::Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	(void)Payload;

	const float AttackSpeed = RuntimeContext.GetAttributeValue(UMAAttributeSet::GetAttackSpeedAttribute(), 1.f);
	const float FinalPlayRate = BasePlayRate * (AttackSpeed > 0.f ? AttackSpeed : 1.f);
	RuntimeContext.SetDesiredMontagePlayRate(FMath::Max(FinalPlayRate, KINDA_SMALL_NUMBER));
}
