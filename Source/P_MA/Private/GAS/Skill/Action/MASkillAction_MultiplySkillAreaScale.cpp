#include "GAS/Skill/Action/MASkillAction_MultiplySkillAreaScale.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_MultiplySkillAreaScale::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Scopes);
	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Writer.IsValid()) return;

	const FGameplayTag AreaScaleTag = UMAAbilitySystemStatics::GetSkillAreaScaleTag();
	const float SafeMultiplier = FMath::Max(Multiplier, 0.f);
	if (!Payloads.Writer.MultiplyScalar(EMASkillPayloadScope::Module, AreaScaleTag, SafeMultiplier))
	{
		Payloads.Writer.SetScalar(EMASkillPayloadScope::Module, AreaScaleTag, SafeMultiplier);
	}
}
