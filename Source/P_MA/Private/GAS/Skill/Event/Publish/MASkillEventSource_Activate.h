#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Publish/MASkillEventSource.h"
#include "MASkillEventSource_Activate.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_Activate : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_Activate()
	{
		EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.Activate"));
	}

	virtual void StartSource(UMASkillAbility* SkillAbility) override;
};
