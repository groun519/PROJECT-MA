#pragma once

#include "CoreMinimal.h"
#include "MASkillModuleTypes.generated.h"

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EMASkillModuleType : uint8
{
	None   = 0 UMETA(Hidden),
	Module = 1 << 0,
	Item   = 1 << 1,
	Sub    = 1 << 2
};
ENUM_CLASS_FLAGS(EMASkillModuleType);
