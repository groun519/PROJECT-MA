#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_DamageDealt.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_DamageDealt : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_DamageDealt();
};
