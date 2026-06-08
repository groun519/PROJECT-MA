#include "GAS/Skill/Action/MASkillAction_SetMontagePlayRateByAttackSpeed.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Step/MASkillStepManager.h"

void UMASkillAction_SetMontagePlayRateByAttackSpeed::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData&,
	const FMASkillEventScopes&)
{
	float AttackSpeed = 1.f;
	if (UAbilitySystemComponent* AbilitySystemComponent = OwnerAbility.GetAbilitySystemComponentFromActorInfo())
	{
		AttackSpeed = AbilitySystemComponent->GetNumericAttribute(UMAAttributeSet::GetAttackSpeedAttribute());
	}

	float SkillAttackSpeedMultiplier = 1.f;
	OwnerAbility.GetAssembledModulePayloadStore().TryGetScalar(
		UMAAbilitySystemStatics::GetSkillAttackSpeedMultiplierTag(),
		SkillAttackSpeedMultiplier);

	const float FinalPlayRate = BasePlayRate
		* (AttackSpeed > 0.f ? AttackSpeed : 1.f)
		* FMath::Max(SkillAttackSpeedMultiplier, KINDA_SMALL_NUMBER);
	if (UMASkillStepManager* StepManager = OwnerAbility.GetStepManager())
		StepManager->SetDesiredMontagePlayRate(FMath::Max(FinalPlayRate, KINDA_SMALL_NUMBER));
}
