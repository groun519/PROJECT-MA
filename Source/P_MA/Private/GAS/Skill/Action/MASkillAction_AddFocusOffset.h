#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_AddFocusOffset.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Add Focus Offset")
class P_MA_API UMASkillAction_AddFocusOffset : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_AddFocusOffset() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Focus")
	float FocusOffset = 0.f;
};
