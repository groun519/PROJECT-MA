#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Publish/MASkillEventSource.h"
#include "MASkillEventSource_End.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_End : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_End()
	{
		EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.End"));
	}

	virtual void StartSource(UMASkillAbility* SkillAbility) override;
	virtual void StopSource() override;

private:
	UPROPERTY(Transient)
	bool bStarted = false;
};
