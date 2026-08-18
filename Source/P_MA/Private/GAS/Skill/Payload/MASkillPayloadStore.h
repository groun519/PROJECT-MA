#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "MASkillPayloadStore.generated.h"

enum class EMASkillPayloadScope : uint8
{
	Module,
	Skill
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillPayloadStore
{
	GENERATED_BODY()

public:
	void Reset()
	{
		RemoveValuesNotKeptOnReset(Scalars);
		RemoveValuesNotKeptOnReset(Vectors);
		RemoveValuesNotKeptOnReset(Objects);
		RemoveValuesNotKeptOnReset(Structs);
	}

	void SetScalar(
		const FGameplayTag& Key,
		float Value,
		bool bKeepValueOnPayloadReset = false)
	{
		if (!Key.IsValid()) return;
		Scalars.FindOrAdd(Key) = Value;
		SetKeepValueOnPayloadReset(Key, bKeepValueOnPayloadReset);
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

	bool AddScalar(const FGameplayTag& Key, float Value, TOptional<float> MinimumValue = {})
	{
		float* CurrentValue = Key.IsValid() ? Scalars.Find(Key) : nullptr;
		if (!CurrentValue) return false;

		*CurrentValue += Value;
		if (MinimumValue.IsSet())
		{
			*CurrentValue = FMath::Max(*CurrentValue, MinimumValue.GetValue());
		}

		return true;
	}

	bool MultiplyScalar(const FGameplayTag& Key, float Value)
	{
		float* CurrentValue = Key.IsValid() ? Scalars.Find(Key) : nullptr;
		if (!CurrentValue) return false;

		*CurrentValue *= Value;
		return true;
	}

	void SetVector(
		const FGameplayTag& Key,
		const FVector& Value,
		bool bKeepValueOnPayloadReset = false)
	{
		if (!Key.IsValid()) return;
		Vectors.FindOrAdd(Key) = Value;
		SetKeepValueOnPayloadReset(Key, bKeepValueOnPayloadReset);
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

	void SetObject(
		const FGameplayTag& Key,
		UObject* Value,
		bool bKeepValueOnPayloadReset = false)
	{
		if (!Key.IsValid()) return;
		Objects.FindOrAdd(Key) = Value;
		SetKeepValueOnPayloadReset(Key, bKeepValueOnPayloadReset);
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
	void SetStruct(
		const FGameplayTag& Key,
		const StructType& Value,
		bool bKeepValueOnPayloadReset = false)
	{
		if (!Key.IsValid()) return;

		FInstancedStruct& StructValue = Structs.FindOrAdd(Key);
		StructValue.InitializeAs<StructType>(Value);
		SetKeepValueOnPayloadReset(Key, bKeepValueOnPayloadReset);
	}

	void SetStructValue(
		const FGameplayTag& Key,
		const FInstancedStruct& Value,
		bool bKeepValueOnPayloadReset = false)
	{
		if (!Key.IsValid() || !Value.IsValid()) return;

		Structs.FindOrAdd(Key) = Value;
		SetKeepValueOnPayloadReset(Key, bKeepValueOnPayloadReset);
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
	const StructType* FindStruct(const FGameplayTag& Key) const
	{
		if (!Key.IsValid()) return nullptr;

		const FInstancedStruct* StructValue = Structs.Find(Key);
		return StructValue && StructValue->GetScriptStruct() == StructType::StaticStruct()
			? StructValue->GetPtr<StructType>()
			: nullptr;
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
	void SetKeepValueOnPayloadReset(const FGameplayTag& Key, bool bKeepValue)
	{
		if (bKeepValue)
		{
			KeysKeptOnReset.Add(Key);
		}
		else
		{
			KeysKeptOnReset.Remove(Key);
		}
	}

	template <typename ValueType>
	void RemoveValuesNotKeptOnReset(TMap<FGameplayTag, ValueType>& Values)
	{
		for (auto It = Values.CreateIterator(); It; ++It)
		{
			if (!KeysKeptOnReset.Contains(It.Key())) It.RemoveCurrent();
		}
	}

	UPROPERTY(Transient)
	TMap<FGameplayTag, float> Scalars;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FVector> Vectors;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UObject>> Objects;

	UPROPERTY(Transient)
	TMap<FGameplayTag, FInstancedStruct> Structs;

	UPROPERTY(Transient)
	TSet<FGameplayTag> KeysKeptOnReset;
};
