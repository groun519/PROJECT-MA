#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "MASkillModuleAddonRuntimeData.generated.h"

USTRUCT()
struct FMASkillModuleAddonRuntimeData
{
	GENERATED_BODY()

	// Each addon owns one unique runtime data struct type.
	// Keep multiple values for that addon inside the same struct.
	void Reset()
	{
		Items.Reset();
	}

	template<typename DataType>
	const DataType* Find() const
	{
		for (const FInstancedStruct& Item : Items)
		{
			if (const DataType* Data = Item.GetPtr<DataType>())
			{
				return Data;
			}
		}
		return nullptr;
	}

	template<typename DataType>
	DataType* FindMutable()
	{
		for (FInstancedStruct& Item : Items)
		{
			if (DataType* Data = Item.GetMutablePtr<DataType>())
			{
				return Data;
			}
		}
		return nullptr;
	}

	template<typename DataType>
	DataType& FindOrAdd()
	{
		if (DataType* Data = FindMutable<DataType>())
		{
			return *Data;
		}

		Items.Add(FInstancedStruct::Make<DataType>());
		return Items.Last().GetMutable<DataType>();
	}

	template<typename DataType, typename MutatorType>
	bool Modify(MutatorType&& Mutator)
	{
		DataType* Data = FindMutable<DataType>();
		if (!Data) return false;
		return Mutator(*Data);
	}

private:
	UPROPERTY()
	TArray<FInstancedStruct> Items;
};
