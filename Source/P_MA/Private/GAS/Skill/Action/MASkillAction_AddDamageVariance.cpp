#include "GAS/Skill/Action/MASkillAction_AddDamageVariance.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillAction_AddDamageVariance::Execute(
	UMASkillAbility&,
	const FGameplayEventData&,
	const FMASkillEventScopes& Scopes)
{
	if (!Scopes.EventScope) return;

	FMASkillPayloadStore& PayloadStore = Scopes.EventScope->GetPayloadStore();
	const FGameplayTag DamageVarianceTag = UMAAbilitySystemStatics::GetDamageVarianceTag();

	float CurrentVariance = 0.f;
	PayloadStore.TryGetScalar(DamageVarianceTag, CurrentVariance);
	PayloadStore.SetScalar(DamageVarianceTag, FMath::Max(0.f, CurrentVariance + VarianceAdditive));
}
