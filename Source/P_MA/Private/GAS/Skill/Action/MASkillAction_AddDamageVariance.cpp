#include "GAS/Skill/Action/MASkillAction_AddDamageVariance.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

void UMASkillAction_AddDamageVariance::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Scopes);
	FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.IsValid()) return;
	const FGameplayTag DamageVarianceTag = UMAAbilitySystemStatics::GetDamageVarianceTag();

	float CurrentVariance = 0.f;
	Payloads.TryGetScalar(DamageVarianceTag, CurrentVariance);
	Payloads.SetScalar(
		EMASkillPayloadWriteScope::Skill,
		DamageVarianceTag,
		FMath::Max(0.f, CurrentVariance + VarianceAdditive));
}
