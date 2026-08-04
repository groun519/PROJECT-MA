#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_MontageStart.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_MontageStart : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_MontageStart()
	{
		EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.MontageStart"));
	}
};
