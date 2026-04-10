#include "GAS/Skill/Action/MASkillAction_SetMontagePlayRateByAttackSpeed.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillAction_SetMontagePlayRateByAttackSpeed::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext&, const FGameplayEventData&)
{
	float AttackSpeed = 1.f;
	if (UAbilitySystemComponent* AbilitySystemComponent = OwnerAbility.GetAbilitySystemComponentFromActorInfo())
	{
		AttackSpeed = AbilitySystemComponent->GetNumericAttribute(UMAAttributeSet::GetAttackSpeedAttribute());
	}

	const float FinalPlayRate = BasePlayRate * (AttackSpeed > 0.f ? AttackSpeed : 1.f);
	OwnerAbility.SetDesiredMontagePlayRate(FMath::Max(FinalPlayRate, KINDA_SMALL_NUMBER));
}
