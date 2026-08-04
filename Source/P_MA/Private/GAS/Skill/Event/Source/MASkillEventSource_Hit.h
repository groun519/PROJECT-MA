#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_Hit.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_Hit : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_Hit();
};
