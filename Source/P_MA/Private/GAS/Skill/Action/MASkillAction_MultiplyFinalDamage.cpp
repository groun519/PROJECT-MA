#include "GAS/Skill/Action/MASkillAction_MultiplyFinalDamage.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

void UMASkillAction_MultiplyFinalDamage::Execute(
	UMASkillAbility&,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;
	const FGameplayTag FinalDamageMultiplierTag = UMAAbilitySystemStatics::GetFinalDamageMultiplierTag();

	float CurrentMultiplier = 1.f;
	Payloads.TryGetScalar(FinalDamageMultiplierTag, CurrentMultiplier);
	Payloads.SetScalar(EMASkillPayloadWriteScope::Skill, FinalDamageMultiplierTag, CurrentMultiplier * Multiplier);
}
