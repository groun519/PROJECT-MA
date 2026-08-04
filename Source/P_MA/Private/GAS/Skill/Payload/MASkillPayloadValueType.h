#pragma once

#include "CoreMinimal.h"
#include "MASkillPayloadValueType.generated.h"

UENUM(BlueprintType)
enum class EMASkillPayloadValueType : uint8
{
	Scalar,
	Vector,
	Object,
	Struct,
};
