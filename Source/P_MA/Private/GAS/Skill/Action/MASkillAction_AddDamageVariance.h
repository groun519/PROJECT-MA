#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_AddDamageVariance.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Add Damage Variance")
class P_MA_API UMASkillAction_AddDamageVariance : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_AddDamageVariance() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(ClampMin="-1.0", UIMin="-1.0"))
	float VarianceAdditive = 0.1f;
};
