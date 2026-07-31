#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_MultiplySkillAttackSpeed.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Multiply Skill Attack Speed")
class P_MA_API UMASkillAction_MultiplySkillAttackSpeed : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_MultiplySkillAttackSpeed() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Animation", meta=(ClampMin="0.01"))
	float Multiplier = 1.f;
};
