#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "MASkillEventSource_ModuleStackChanged.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillEventSource_ModuleStackChanged : public UMASkillEventSource
{
	GENERATED_BODY()

public:
	UMASkillEventSource_ModuleStackChanged()
	{
		EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Module.StackChanged"));
	}
};
