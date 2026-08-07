#include "GAS/Skill/Payload/MASkillPayloadReader.h"

FMASkillPayloadReader::FMASkillPayloadReader(
	const FMASkillPayloadStore* InEventPayloads,
	const FMASkillPayloadStore* InSkillPayloads,
	const FMASkillPayloadStore* InModulePayloads)
	: EventPayloads(InEventPayloads)
	, SkillPayloads(InSkillPayloads)
	, ModulePayloads(InModulePayloads)
{
}

bool FMASkillPayloadReader::TryGetScalar(const FGameplayTag& Key, float& OutValue) const
{
	return TryRead([&Key, &OutValue](const FMASkillPayloadStore& Store)
	{
		return Store.TryGetScalar(Key, OutValue);
	});
}

bool FMASkillPayloadReader::TryGetScalar(
	EMASkillPayloadScope Scope,
	const FGameplayTag& Key,
	float& OutValue) const
{
	const FMASkillPayloadStore* Store = Scope == EMASkillPayloadScope::Skill
		? SkillPayloads
		: ModulePayloads;
	return Store && Store->TryGetScalar(Key, OutValue);
}

float FMASkillPayloadReader::GetScalarProduct(const FGameplayTag& Key) const
{
	float Product = 1.f;
	ForEachStore([&Key, &Product](const FMASkillPayloadStore& Store)
	{
		float Value = 0.f;
		if (Store.TryGetScalar(Key, Value)) Product *= Value;
	});
	return Product;
}

float FMASkillPayloadReader::GetScalarSum(const FGameplayTag& Key) const
{
	float Sum = 0.f;
	ForEachStore([&Key, &Sum](const FMASkillPayloadStore& Store)
	{
		float Value = 0.f;
		if (Store.TryGetScalar(Key, Value)) Sum += Value;
	});
	return Sum;
}

bool FMASkillPayloadReader::TryGetVector(const FGameplayTag& Key, FVector& OutValue) const
{
	return TryRead([&Key, &OutValue](const FMASkillPayloadStore& Store)
	{
		return Store.TryGetVector(Key, OutValue);
	});
}

bool FMASkillPayloadReader::TryGetObject(const FGameplayTag& Key, UObject*& OutValue) const
{
	return TryRead([&Key, &OutValue](const FMASkillPayloadStore& Store)
	{
		return Store.TryGetObject(Key, OutValue);
	});
}
