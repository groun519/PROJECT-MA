#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_AddFocusOverflowCriticalDamage.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Add Focus Overflow Critical Damage")
class P_MA_API UMASkillAction_AddFocusOverflowCriticalDamage : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_AddFocusOverflowCriticalDamage()
	{
		SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub;
	}

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Critical Damage")
	float CriticalDamageRate = 0.5f;
};
