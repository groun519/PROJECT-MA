#include "GAS/Skill/Action/MASkillAction_MultiplyFinalDamage.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillAction_MultiplyFinalDamage::Execute(
	UMASkillAbility&,
	const FGameplayEventData&,
	UMASkillModuleInstance*,
	UMASkillModuleInstance* EventOwnerScope)
{
	if (!EventOwnerScope) return;

	FMASkillPayloadStore& PayloadStore = EventOwnerScope->GetPayloadStore();
	const FGameplayTag FinalDamageMultiplierTag = UMAAbilitySystemStatics::GetFinalDamageMultiplierTag();

	float CurrentMultiplier = 1.f;
	PayloadStore.TryGetScalar(FinalDamageMultiplierTag, CurrentMultiplier);
	PayloadStore.SetScalar(FinalDamageMultiplierTag, CurrentMultiplier * Multiplier);
}
