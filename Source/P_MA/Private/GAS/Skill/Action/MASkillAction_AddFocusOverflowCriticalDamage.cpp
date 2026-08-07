#include "GAS/Skill/Action/MASkillAction_AddFocusOverflowCriticalDamage.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_AddFocusOverflowCriticalDamage::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	UAbilitySystemComponent* AbilitySystemComponent = Ability->GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent) return;

	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	if (!Payloads.Writer.IsValid()) return;

	float FocusOffset = 0.f;
	Payloads.Reader.TryGetScalar(UMAAbilitySystemStatics::GetSkillFocusOffsetTag(), FocusOffset);
	const float Focus = AbilitySystemComponent->GetNumericAttribute(UMAAttributeSet::GetFocusAttribute()) + FocusOffset;
	const float CriticalDamageOffset = FMath::Max(Focus - 1.f, 0.f) * CriticalDamageRate;
	if (FMath::IsNearlyZero(CriticalDamageOffset)) return;

	const FGameplayTag CriticalDamageOffsetTag = UMAAbilitySystemStatics::GetSkillCriticalDamageOffsetTag();
	if (!Payloads.Writer.AddScalar(EMASkillPayloadScope::Skill, CriticalDamageOffsetTag, CriticalDamageOffset))
	{
		Payloads.Writer.SetScalar(EMASkillPayloadScope::Skill, CriticalDamageOffsetTag, CriticalDamageOffset);
	}
}
