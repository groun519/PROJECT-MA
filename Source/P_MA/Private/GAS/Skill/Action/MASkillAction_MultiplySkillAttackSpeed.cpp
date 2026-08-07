#include "GAS/Skill/Action/MASkillAction_MultiplySkillAttackSpeed.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"
#include "GAS/Skill/Sequence/MASkillSequenceRuntime.h"

void UMASkillAction_MultiplySkillAttackSpeed::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Writer.IsValid()) return;
	const FGameplayTag AttackSpeedMultiplierTag = UMAAbilitySystemStatics::GetSkillAttackSpeedMultiplierTag();
	const float SafeMultiplier = FMath::Max(Multiplier, KINDA_SMALL_NUMBER);

	if (!Payloads.Writer.MultiplyScalar(EMASkillPayloadScope::Module, AttackSpeedMultiplierTag, SafeMultiplier))
	{
		Payloads.Writer.SetScalar(EMASkillPayloadScope::Module, AttackSpeedMultiplierTag, SafeMultiplier);
	}

	if (UMASkillSequenceRuntime* SequenceRuntime = Ability->GetSequenceRuntime())
	{
		SequenceRuntime->RefreshPlayRate();
	}
}
