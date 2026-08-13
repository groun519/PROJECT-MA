#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_DamageApplied.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_DamageApplied : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_DamageApplied()
	{
		EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.DamageApplied"));
	}
};
