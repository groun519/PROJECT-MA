#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "MASkillPayloadStore.generated.h"

USTRUCT(BlueprintType)
struct P_MA_API FMASkillPayloadStore
{
	GENERATED_BODY()

public:
	void Reset()
	{
		Scalars.Reset();
		Vectors.Reset();
		Objects.Reset();
		Structs.Reset();
	}

	void SetScalar(const FGameplayTag& Key, float Value)
	{
		if (!Key.IsValid()) return;
		Scalars.FindOrAdd(Key) = Value;
	}

	bool TryGetScalar(const FGameplayTag& Key, float& OutValue) const
	{
		if (!Key.IsValid()) return false;

		if (const float* Value = Scalars.Find(Key))
		{
			OutValue = *Value;
			return true;
		}

		return false;
	}

	void SetVector(const FGameplayTag& Key, const FVector& Value)
	{
		if (!Key.IsValid()) return;
		Vectors.FindOrAdd(Key) = Value;
	}

	bool TryGetVector(const FGameplayTag& Key, FVector& OutValue) const
	{
		if (!Key.IsValid()) return false;

		if (const FVector* Value = Vectors.Find(Key))
		{
			OutValue = *Value;
			return true;
		}

		return false;
	}

	void SetObject(const FGameplayTag& Key, UObject* Value)
	{
		if (!Key.IsValid()) return;
		Objects.FindOrAdd(Key) = Value;
	}

	bool TryGetObject(const FGameplayTag& Key, UObject*& OutValue) const
	{
		if (!Key.IsValid()) return false;

		if (const TObjectPtr<UObject>* Value = Objects.Find(Key))
		{
			OutValue = Value->Get();
			return true;
		}

		return false;
	}

	template <typename StructType>
	void SetStruct(const FGameplayTag& Key, const StructType& Value)
	{
		if (!Key.IsValid()) return;

		FInstancedStruct& StructValue = Structs.FindOrAdd(Key);
		StructValue.InitializeAs<StructType>(Value);
	}

	void SetStructValue(const FGameplayTag& Key, const FInstancedStruct& Value)
	{
		if (!Key.IsValid() || !Value.IsValid()) return;

		Structs.FindOrAdd(Key) = Value;
	}

	template <typename StructType>
	bool TryGetStruct(const FGameplayTag& Key, StructType& OutValue) const
	{
		if (!Key.IsValid()) return false;

		const FInstancedStruct* StructValue = Structs.Find(Key);
		if (!StructValue || !StructValue->IsValid()) return false;
		if (StructValue->GetScriptStruct() != StructType::StaticStruct()) return false;

		if (const StructType* Value = StructValue->GetPtr<StructType>())
		{
			OutValue = *Value;
			return true;
		}

		return false;
	}

	template <typename StructType>
	void FindStructsByTag(
		const FGameplayTag& Key,
		bool bExactTagMatch,
		TArray<TPair<FGameplayTag, StructType>>& OutValues) const
	{
		OutValues.Reset();
		if (!Key.IsValid()) return;

		if (bExactTagMatch)
		{
			StructType Value;
			if (TryGetStruct(Key, Value))
			{
				OutValues.Emplace(Key, MoveTemp(Value));
			}

			return;
		}

		for (const TPair<FGameplayTag, FInstancedStruct>& Pair : Structs)
		{
			if (!Pair.Key.MatchesTag(Key)) continue;
			if (!Pair.Value.IsValid()) continue;
			if (Pair.Value.GetScriptStruct() != StructType::StaticStruct()) continue;

			if (const StructType* Value = Pair.Value.GetPtr<StructType>())
			{
				OutValues.Emplace(Pair.Key, *Value);
			}
		}
	}

private:
	UPROPERTY(Transient)
	TMap<FGameplayTag, float> Scalars;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FVector> Vectors;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UObject>> Objects;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FInstancedStruct> Structs;
};

