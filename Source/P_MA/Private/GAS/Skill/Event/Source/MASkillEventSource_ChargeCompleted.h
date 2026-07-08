#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_ChargeCompleted.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_ChargeCompleted : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_ChargeCompleted();
};
