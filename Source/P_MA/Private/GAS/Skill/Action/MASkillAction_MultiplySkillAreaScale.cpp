#include "GAS/Skill/Action/MASkillAction_MultiplySkillAreaScale.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

void UMASkillAction_MultiplySkillAreaScale::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;

	const FGameplayTag AreaScaleTag = UMAAbilitySystemStatics::GetSkillAreaScaleTag();
	float CurrentScale = 1.f;
	Payloads.TryGetScalar(AreaScaleTag, CurrentScale);
	Payloads.SetScalar(EMASkillPayloadWriteScope::Skill, AreaScaleTag, CurrentScale * FMath::Max(Multiplier, 0.f));
}
