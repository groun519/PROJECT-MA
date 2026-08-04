#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_MultiplySkillAreaScale.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Multiply Skill Area Scale")
class P_MA_API UMASkillAction_MultiplySkillAreaScale : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_MultiplySkillAreaScale() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Area", meta=(ClampMin="0.0"))
	float Multiplier = 1.f;
};
