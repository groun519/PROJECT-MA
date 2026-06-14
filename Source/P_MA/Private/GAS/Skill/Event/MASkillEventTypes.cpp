#include "GAS/Skill/Event/MASkillEventTypes.h"

#include "GAS/Skill/Module/MASkillModuleInstance.h"

namespace
{
	const FGameplayTag EventMagnitudeTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Event.Magnitude"));
	const FGameplayTag EventTargetDataTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Event.TargetData"));
}

FMASkillPayloadAccessor FMASkillScopes::GetPayloadAccess(const FMASkillPayloadStore* EventPayloads) const
{
	return FMASkillPayloadAccessor(
		EventPayloads,
		Skill ? &Skill->GetPayloadStore() : nullptr,
		Module ? &Module->GetPayloadStore() : nullptr);
}

UMASkillRuntimeRegistry& FMASkillScopes::GetRuntimeRegistry() const
{
	check(Skill);
	UMASkillRuntimeRegistry* RuntimeRegistry = Skill->GetRuntimeRegistry();
	check(RuntimeRegistry);
	return *RuntimeRegistry;
}

void FMASkillEvent::SetMagnitude(float Magnitude)
{
	Payloads.SetScalar(EventMagnitudeTag, Magnitude);
}

float FMASkillEvent::GetMagnitude() const
{
	float Magnitude = 0.f;
	Payloads.TryGetScalar(EventMagnitudeTag, Magnitude);
	return Magnitude;
}

void FMASkillEvent::SetTargetData(const FGameplayAbilityTargetDataHandle& TargetData)
{
	Payloads.SetStruct(EventTargetDataTag, TargetData);
}

const FGameplayAbilityTargetDataHandle* FMASkillEvent::GetTargetData() const
{
	return Payloads.FindStruct<FGameplayAbilityTargetDataHandle>(EventTargetDataTag);
}
