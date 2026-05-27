#include "GAS/Skill/Event/Publish/MASkillEventSource_DamageDealt.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

UMASkillEventSource_DamageDealt::UMASkillEventSource_DamageDealt()
{
	EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.DamageDealt"));
}

void UMASkillEventSource_DamageDealt::HandleSourceEvent(
	UMASkillAbility& SkillAbility,
	UMASkillModuleInstance& InEventScope,
	const FGameplayTag& SourceEventTag,
	const FGameplayEventData& EventData) const
{
	if (SourceEventTag != EmittedTag || EventData.EventMagnitude <= 0.f) return;

	FMASkillPayloadStore& PayloadStore = InEventScope.GetPayloadStore();
	PayloadStore.SetScalar(UMAAbilitySystemStatics::GetAppliedDamageTag(), EventData.EventMagnitude);
	PayloadStore.SetObject(UMAAbilitySystemStatics::GetDamageTargetTag(), const_cast<AActor*>(EventData.Target.Get()));
	EmitEvent(SkillAbility, InEventScope, EventData);
}
