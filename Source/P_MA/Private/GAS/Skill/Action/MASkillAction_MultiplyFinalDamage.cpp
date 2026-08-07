#include "GAS/Skill/Action/MASkillAction_MultiplyFinalDamage.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_MultiplyFinalDamage::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Scopes);
	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Writer.IsValid()) return;
	const FGameplayTag FinalDamageMultiplierTag = UMAAbilitySystemStatics::GetFinalDamageMultiplierTag();

	float AppliedMultiplier = Multiplier;
	if (MultiplierPayloadTag.IsValid())
	{
		float PayloadMultiplier = 0.f;
		if (!Payloads.Reader.TryGetScalar(MultiplierPayloadTag, PayloadMultiplier)) return;
		AppliedMultiplier = PayloadBaseMultiplier + Multiplier * PayloadMultiplier;
	}

	const EMASkillPayloadScope PayloadScope = GetPayloadScope();
	if (!Payloads.Writer.MultiplyScalar(PayloadScope, FinalDamageMultiplierTag, AppliedMultiplier))
	{
		Payloads.Writer.SetScalar(PayloadScope, FinalDamageMultiplierTag, AppliedMultiplier);
	}
}

EMASkillPayloadScope UMASkillAction_MultiplyFinalDamage::GetPayloadScope() const
{
	return EMASkillPayloadScope::Skill;
}

EMASkillPayloadScope UMASkillAction_MultiplyModuleFinalDamage::GetPayloadScope() const
{
	return EMASkillPayloadScope::Module;
}
