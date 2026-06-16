#include "GAS/Skill/Action/MASkillAction_AddFocusOffset.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"

void UMASkillAction_AddFocusOffset::Execute(
	UMASkillAbility&,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	if (!Payloads.IsValid()) return;

	const FGameplayTag FocusOffsetTag = UMAAbilitySystemStatics::GetSkillFocusOffsetTag();
	float CurrentOffset = 0.f;
	Payloads.TryGetScalar(FocusOffsetTag, CurrentOffset);
	Payloads.SetScalar(EMASkillPayloadWriteScope::Skill, FocusOffsetTag, CurrentOffset + FocusOffset);
}
