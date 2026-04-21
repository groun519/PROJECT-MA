#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_Dash.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_Dash : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData& Payload) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Dash", meta=(ClampMin="0.0"))
	float DashSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category="Dash", meta=(ClampMin="0.0"))
	float DashDuration = 0.2f;
};
