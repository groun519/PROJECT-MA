#include "GAS/Skill/Action/MASkillAction_AddDamageVariance.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_AddDamageVariance::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Scopes);
	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Writer.IsValid()) return;
	const FGameplayTag DamageVarianceTag = UMAAbilitySystemStatics::GetDamageVarianceTag();

	if (!Payloads.Writer.AddScalar(EMASkillPayloadScope::Module, DamageVarianceTag, VarianceAdditive, 0.f))
	{
		Payloads.Writer.SetScalar(EMASkillPayloadScope::Module, DamageVarianceTag, FMath::Max(0.f, VarianceAdditive));
	}
}
