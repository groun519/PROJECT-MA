#include "GAS/Skill/Action/MASkillAction_SetMontagePlayRateByAttackSpeed.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Sequence/MASkillSequenceRuntime.h"

void UMASkillAction_SetMontagePlayRateByAttackSpeed::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability);
	float AttackSpeed = 1.f;
	if (UAbilitySystemComponent* AbilitySystemComponent = Ability->GetAbilitySystemComponentFromActorInfo())
	{
		AttackSpeed = AbilitySystemComponent->GetNumericAttribute(UMAAttributeSet::GetAttackSpeedAttribute());
	}

	float SkillAttackSpeedMultiplier = 1.f;
	Ability->GetAssembledModulePayloadStore().TryGetScalar(
		UMAAbilitySystemStatics::GetSkillAttackSpeedMultiplierTag(),
		SkillAttackSpeedMultiplier);

	const float FinalPlayRate = BasePlayRate
		* (AttackSpeed > 0.f ? AttackSpeed : 1.f)
		* FMath::Max(SkillAttackSpeedMultiplier, KINDA_SMALL_NUMBER);
	if (UMASkillSequenceRuntime* SequenceRuntime = Ability->GetSequenceRuntime())
	{
		SequenceRuntime->SetDesiredPlayRate(FinalPlayRate);
	}
}
