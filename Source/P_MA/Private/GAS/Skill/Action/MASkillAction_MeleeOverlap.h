#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_MeleeOverlap.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_MeleeOverlap : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore, const FGameplayEventData& Payload) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;
};
