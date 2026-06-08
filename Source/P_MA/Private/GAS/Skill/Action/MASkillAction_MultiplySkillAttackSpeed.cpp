#include "GAS/Skill/Action/MASkillAction_MultiplySkillAttackSpeed.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Step/MASkillStepManager.h"

void UMASkillAction_MultiplySkillAttackSpeed::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData&,
	const FMASkillEventScopes&)
{
	FMASkillPayloadStore& PayloadStore = OwnerAbility.GetAssembledModulePayloadStore();
	const FGameplayTag AttackSpeedMultiplierTag = UMAAbilitySystemStatics::GetSkillAttackSpeedMultiplierTag();
	const float SafeMultiplier = FMath::Max(Multiplier, KINDA_SMALL_NUMBER);

	float CurrentMultiplier = 1.f;
	PayloadStore.TryGetScalar(AttackSpeedMultiplierTag, CurrentMultiplier);
	PayloadStore.SetScalar(AttackSpeedMultiplierTag, CurrentMultiplier * SafeMultiplier);

	if (UMASkillStepManager* StepManager = OwnerAbility.GetStepManager())
	{
		StepManager->SetDesiredMontagePlayRate(StepManager->GetDesiredMontagePlayRate() * SafeMultiplier);
	}
}
