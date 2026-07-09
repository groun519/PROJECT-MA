#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_ModuleActivationChanged.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_ModuleActivationChanged : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_ModuleActivationChanged()
	{
		EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Module.ActivationChanged"));
	}
};
