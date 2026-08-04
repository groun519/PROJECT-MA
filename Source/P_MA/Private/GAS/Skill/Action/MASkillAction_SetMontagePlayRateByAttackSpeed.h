#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_SetMontagePlayRateByAttackSpeed.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_SetMontagePlayRateByAttackSpeed : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_SetMontagePlayRateByAttackSpeed() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Animation", meta=(ClampMin="0.01"))
	float BasePlayRate = 1.f;
};
