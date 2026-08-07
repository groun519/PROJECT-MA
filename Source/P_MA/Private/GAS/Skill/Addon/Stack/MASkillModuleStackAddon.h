#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "MASkillModuleStackAddon.generated.h"

USTRUCT()
struct FMASkillModuleStackRuntimeData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Stack = 0;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleStackAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	UMASkillModuleStackAddon()
	{
		SupportedModuleTypes = EMASkillModuleType::Module;
	}

	virtual void InitializeRuntimeData(FMASkillModuleAddonRuntimeData& RuntimeData) const override;
	virtual void ApplyPayloadMirror(const FMASkillModuleAddonRuntimeData& RuntimeData, FMASkillPayloadStore& PayloadStore) const override;
	virtual bool TryResolveSocketText(const FMASkillModuleAddonRuntimeData& RuntimeData, FText& OutText) const override;
	int32 ClampStack(int64 Value) const;

private:
	UPROPERTY(EditDefaultsOnly, Category="Stack")
	int32 InitialStack = 0;

	UPROPERTY(EditDefaultsOnly, Category="Stack")
	int32 MinStack = 0;

	UPROPERTY(EditDefaultsOnly, Category="Stack")
	int32 MaxStack = 999;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	bool bShowStackText = true;
};
