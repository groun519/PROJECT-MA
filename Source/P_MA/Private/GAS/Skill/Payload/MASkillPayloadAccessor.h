#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

enum class EMASkillPayloadWriteScope : uint8
{
	Module,
	Skill
};

struct P_MA_API FMASkillPayloadAccessor
{
	FMASkillPayloadAccessor(
		const FMASkillPayloadStore* InEventPayloads,
		const FMASkillPayloadStore* InSkillPayloads,
		const FMASkillPayloadStore* InModulePayloads)
		: EventPayloads(InEventPayloads)
		, SkillPayloads(InSkillPayloads)
		, ModulePayloads(InModulePayloads)
	{
	}

	FMASkillPayloadAccessor(
		const FMASkillPayloadStore* InEventPayloads,
		FMASkillPayloadStore* InSkillPayloads,
		FMASkillPayloadStore* InModulePayloads)
		: EventPayloads(InEventPayloads)
		, SkillPayloads(InSkillPayloads)
		, ModulePayloads(InModulePayloads)
		, SkillWritePayloads(InSkillPayloads)
		, ModuleWritePayloads(InModulePayloads)
	{
	}

	bool IsValid() const { return EventPayloads || SkillPayloads || ModulePayloads; }

	bool TryGetScalar(const FGameplayTag& Key, float& OutValue) const
	{
		return TryRead([&Key, &OutValue](const FMASkillPayloadStore& Store)
		{
			return Store.TryGetScalar(Key, OutValue);
		});
	}

	bool TryGetVector(const FGameplayTag& Key, FVector& OutValue) const
	{
		return TryRead([&Key, &OutValue](const FMASkillPayloadStore& Store)
		{
			return Store.TryGetVector(Key, OutValue);
		});
	}

	bool TryGetObject(const FGameplayTag& Key, UObject*& OutValue) const
	{
		return TryRead([&Key, &OutValue](const FMASkillPayloadStore& Store)
		{
			return Store.TryGetObject(Key, OutValue);
		});
	}

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
		ForEachReadStore([&](const FMASkillPayloadStore& Store)
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

	void SetScalar(EMASkillPayloadWriteScope Scope, const FGameplayTag& Key, float Value)
	{
		if (FMASkillPayloadStore* Store = GetWriteStore(Scope)) Store->SetScalar(Key, Value);
	}

	void SetVector(EMASkillPayloadWriteScope Scope, const FGameplayTag& Key, const FVector& Value)
	{
		if (FMASkillPayloadStore* Store = GetWriteStore(Scope)) Store->SetVector(Key, Value);
	}

	void SetObject(EMASkillPayloadWriteScope Scope, const FGameplayTag& Key, UObject* Value)
	{
		if (FMASkillPayloadStore* Store = GetWriteStore(Scope)) Store->SetObject(Key, Value);
	}

	template <typename StructType>
	void SetStruct(EMASkillPayloadWriteScope Scope, const FGameplayTag& Key, const StructType& Value)
	{
		if (FMASkillPayloadStore* Store = GetWriteStore(Scope)) Store->SetStruct(Key, Value);
	}

private:
	template <typename PredicateType>
	bool TryRead(PredicateType&& Predicate) const
	{
		bool bFound = false;
		ForEachReadStore([&](const FMASkillPayloadStore& Store)
		{
			if (!bFound) bFound = Predicate(Store);
		});
		return bFound;
	}

	template <typename FuncType>
	void ForEachReadStore(FuncType&& Func) const
	{
		if (EventPayloads) Func(*EventPayloads);
		if (SkillPayloads && SkillPayloads != EventPayloads) Func(*SkillPayloads);
		if (ModulePayloads && ModulePayloads != EventPayloads && ModulePayloads != SkillPayloads) Func(*ModulePayloads);
	}

	FMASkillPayloadStore* GetWriteStore(EMASkillPayloadWriteScope Scope) const
	{
		return Scope == EMASkillPayloadWriteScope::Skill ? SkillWritePayloads : ModuleWritePayloads;
	}

	const FMASkillPayloadStore* EventPayloads = nullptr;
	const FMASkillPayloadStore* SkillPayloads = nullptr;
	const FMASkillPayloadStore* ModulePayloads = nullptr;
	FMASkillPayloadStore* SkillWritePayloads = nullptr;
	FMASkillPayloadStore* ModuleWritePayloads = nullptr;
};

