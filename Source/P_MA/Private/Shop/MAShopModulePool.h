#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MAShopModulePool.generated.h"

UCLASS(BlueprintType)
class P_MA_API UMAShopModulePool : public UDataAsset
{
	GENERATED_BODY()

public:
	const TArray<int32>& GetModuleIds() const { return ModuleIds; }

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
	// Per-module appearance weights can extend these entries when shop probability tuning is added.
	UPROPERTY(EditDefaultsOnly, Category="Shop|Modules", meta=(ClampMin="1"))
	TArray<int32> ModuleIds;
};
