#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_ClearIgnoredActors.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Clear Ignored Actors")
class P_MA_API UMASkillAction_ClearIgnoredActors : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload) override;
};
