#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASkillModuleAddon.generated.h"

/**
 * Definition-only extension point for optional module features.
 * Runtime state should stay on the module instance or a dedicated runtime layer.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleAddon : public UObject
{
	GENERATED_BODY()
};
