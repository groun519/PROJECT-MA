#include "GAS/Skill/Event/MASkillEventTypes.h"

#include "GAS/Skill/Module/MASkillModuleInstance.h"

namespace
{
FGameplayTag GetEventMagnitudeTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Data.Event.Magnitude"));
}

FGameplayTag GetEventTargetDataTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Data.Event.TargetData"));
}
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
	Payloads.SetScalar(GetEventMagnitudeTag(), Magnitude);
}

float FMASkillEvent::GetMagnitude() const
{
	float Magnitude = 0.f;
	Payloads.TryGetScalar(GetEventMagnitudeTag(), Magnitude);
	return Magnitude;
}

void FMASkillEvent::SetTargetData(const FGameplayAbilityTargetDataHandle& TargetData)
{
	Payloads.SetStruct(GetEventTargetDataTag(), TargetData);
}

const FGameplayAbilityTargetDataHandle* FMASkillEvent::GetTargetData() const
{
	return Payloads.FindStruct<FGameplayAbilityTargetDataHandle>(GetEventTargetDataTag());
}

FMASkillPayloadAccessor FMASkillEvent::GetPayloadAccess(const FMASkillScopes& BindingScopes) const
{
	return BindingScopes.GetPayloadAccess(Payloads);
}
