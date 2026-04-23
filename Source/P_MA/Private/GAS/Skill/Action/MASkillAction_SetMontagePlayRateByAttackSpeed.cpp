#include "GAS/Skill/Action/MASkillAction_SetMontagePlayRateByAttackSpeed.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Step/MASkillStepManager.h"

void UMASkillAction_SetMontagePlayRateByAttackSpeed::Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData&)
{
	float AttackSpeed = 1.f;
	if (UAbilitySystemComponent* AbilitySystemComponent = OwnerAbility.GetAbilitySystemComponentFromActorInfo())
	{
		AttackSpeed = AbilitySystemComponent->GetNumericAttribute(UMAAttributeSet::GetAttackSpeedAttribute());
	}

	const float FinalPlayRate = BasePlayRate * (AttackSpeed > 0.f ? AttackSpeed : 1.f);
	if (UMASkillStepManager* StepManager = OwnerAbility.GetStepManager())
		StepManager->SetDesiredMontagePlayRate(FMath::Max(FinalPlayRate, KINDA_SMALL_NUMBER));
}
