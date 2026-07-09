#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASkillModuleAddon.generated.h"

struct FMASkillModuleAddonRuntimeData;
struct FMASkillPayloadStore;

/**
 * Definition-only extension point for optional module features.
 * Runtime state stays on the module instance as typed replicated data.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleAddon : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeRuntimeData(FMASkillModuleAddonRuntimeData& RuntimeData) const {}
	virtual void ApplyPayloadMirror(const FMASkillModuleAddonRuntimeData& RuntimeData, FMASkillPayloadStore& PayloadStore) const {}
	virtual bool TryResolveSocketText(const FMASkillModuleAddonRuntimeData& RuntimeData, FText& OutText) const { return false; }
};
