#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_MeleeOverlap.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_MeleeOverlap : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FMADamageExecutionConfig DamageConfig;
};
