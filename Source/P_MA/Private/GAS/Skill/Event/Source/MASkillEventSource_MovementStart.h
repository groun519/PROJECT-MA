#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_MovementStart.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_MovementStart : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_MovementStart()
	{
		EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.MovementStart"));
	}
};
