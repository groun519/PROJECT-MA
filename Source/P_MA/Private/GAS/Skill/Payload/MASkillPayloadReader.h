#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

struct P_MA_API FMASkillPayloadReader
{
	FMASkillPayloadReader() = default;
	FMASkillPayloadReader(
		const FMASkillPayloadStore* InEventPayloads,
		const FMASkillPayloadStore* InSkillPayloads,
		const FMASkillPayloadStore* InModulePayloads);

	bool IsValid() const { return EventPayloads || SkillPayloads || ModulePayloads; }

	bool TryGetScalar(const FGameplayTag& Key, float& OutValue) const;
	bool TryGetScalar(EMASkillPayloadScope Scope, const FGameplayTag& Key, float& OutValue) const;
	float GetScalarProduct(const FGameplayTag& Key) const;
	float GetScalarSum(const FGameplayTag& Key) const;
	bool TryGetVector(const FGameplayTag& Key, FVector& OutValue) const;
	bool TryGetObject(const FGameplayTag& Key, UObject*& OutValue) const;

	template <typename StructType>
	bool TryGetStruct(const FGameplayTag& Key, StructType& OutValue) const
	{
		return TryRead([&Key, &OutValue](const FMASkillPayloadStore& Store)
		{
			return Store.TryGetStruct(Key, OutValue);
		});
	}

	template <typename StructType>
	void FindStructsByTag(
		const FGameplayTag& Key,
		bool bExactTagMatch,
		TArray<TPair<FGameplayTag, StructType>>& OutValues) const
	{
		OutValues.Reset();
		TSet<FGameplayTag> FoundTags;
		ForEachStore([&](const FMASkillPayloadStore& Store)
		{
			TArray<TPair<FGameplayTag, StructType>> StoreValues;
			Store.FindStructsByTag(Key, bExactTagMatch, StoreValues);
			for (TPair<FGameplayTag, StructType>& Pair : StoreValues)
			{
				if (FoundTags.Contains(Pair.Key)) continue;

				FoundTags.Add(Pair.Key);
				OutValues.Add(MoveTemp(Pair));
			}
		});
	}

private:
	template <typename PredicateType>
	bool TryRead(PredicateType&& Predicate) const
	{
		bool bFound = false;
		ForEachStore([&](const FMASkillPayloadStore& Store)
		{
			if (!bFound) bFound = Predicate(Store);
		});
		return bFound;
	}

	template <typename FuncType>
	void ForEachStore(FuncType&& Func) const
	{
		if (EventPayloads) Func(*EventPayloads);
		if (SkillPayloads && SkillPayloads != EventPayloads) Func(*SkillPayloads);
		if (ModulePayloads && ModulePayloads != EventPayloads && ModulePayloads != SkillPayloads) Func(*ModulePayloads);
	}

	const FMASkillPayloadStore* EventPayloads = nullptr;
	const FMASkillPayloadStore* SkillPayloads = nullptr;
	const FMASkillPayloadStore* ModulePayloads = nullptr;
};
