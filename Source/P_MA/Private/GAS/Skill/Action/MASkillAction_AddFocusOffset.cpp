#include "GAS/Skill/Action/MASkillAction_AddFocusOffset.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_AddFocusOffset::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Scopes);
	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Writer.IsValid()) return;

	const FGameplayTag FocusOffsetTag = UMAAbilitySystemStatics::GetSkillFocusOffsetTag();
	if (!Payloads.Writer.AddScalar(EMASkillPayloadScope::Skill, FocusOffsetTag, FocusOffset))
	{
		Payloads.Writer.SetScalar(EMASkillPayloadScope::Skill, FocusOffsetTag, FocusOffset);
	}
}
