#include "GAS/Skill/Action/MASkillAction_MultiplySkillAttackSpeed.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"
#include "GAS/Skill/Sequence/MASkillSequenceRuntime.h"

void UMASkillAction_MultiplySkillAttackSpeed::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.IsValid()) return;
	const FGameplayTag AttackSpeedMultiplierTag = UMAAbilitySystemStatics::GetSkillAttackSpeedMultiplierTag();
	const float SafeMultiplier = FMath::Max(Multiplier, KINDA_SMALL_NUMBER);

	float CurrentMultiplier = 1.f;
	Payloads.TryGetScalar(AttackSpeedMultiplierTag, CurrentMultiplier);
	Payloads.SetScalar(EMASkillPayloadWriteScope::Skill, AttackSpeedMultiplierTag, CurrentMultiplier * SafeMultiplier);

	if (UMASkillSequenceRuntime* SequenceRuntime = Ability->GetSequenceRuntime())
	{
		SequenceRuntime->SetDesiredPlayRate(SequenceRuntime->GetDesiredPlayRate() * SafeMultiplier);
	}
}
