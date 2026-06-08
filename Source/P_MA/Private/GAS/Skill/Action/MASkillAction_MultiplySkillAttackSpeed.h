#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_MultiplySkillAttackSpeed.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Multiply Skill Attack Speed")
class P_MA_API UMASkillAction_MultiplySkillAttackSpeed : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData& EventData, const FMASkillEventScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Animation", meta=(ClampMin="0.01"))
	float Multiplier = 1.f;
};
