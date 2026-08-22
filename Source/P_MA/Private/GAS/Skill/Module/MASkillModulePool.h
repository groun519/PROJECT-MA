#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MASkillModulePool.generated.h"

UCLASS(BlueprintType)
class P_MA_API UMASkillModulePool : public UDataAsset
{
	GENERATED_BODY()

public:
	const TArray<int32>& GetModuleIds() const { return ModuleIds; }
	int32 SelectRandomModuleId() const
	{
		return ModuleIds.IsEmpty() ? 0 : ModuleIds[FMath::RandHelper(ModuleIds.Num())];
	}

#if WITH_EDITOR
	bool AddModuleId(const int32 ModuleId)
	{
		if (ModuleId <= 0 || ModuleIds.Contains(ModuleId)) return false;
		ModuleIds.Add(ModuleId);
		return true;
	}

	bool RemoveModuleId(const int32 ModuleId)
	{
		return ModuleIds.Remove(ModuleId) > 0;
	}
#endif

private:
	UPROPERTY(EditDefaultsOnly, Category="Pool", meta=(ClampMin="1"))
	TArray<int32> ModuleIds;
};
