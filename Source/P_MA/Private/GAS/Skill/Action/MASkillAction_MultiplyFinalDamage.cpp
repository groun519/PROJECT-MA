#include "GAS/Skill/Action/MASkillAction_MultiplyFinalDamage.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillAction_MultiplyFinalDamage::Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData&, UMASkillModuleInstance*)
{
	FMASkillPayloadStore& PayloadStore = OwnerAbility.GetAssembledModulePayloadStore();
	const FGameplayTag FinalDamageMultiplierTag = UMAAbilitySystemStatics::GetFinalDamageMultiplierTag();

	float CurrentMultiplier = 1.f;
	PayloadStore.TryGetScalar(FinalDamageMultiplierTag, CurrentMultiplier);
	PayloadStore.SetScalar(FinalDamageMultiplierTag, CurrentMultiplier * Multiplier);
}
