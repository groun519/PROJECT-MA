#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_ShatterFrozenTarget.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Shatter Frozen Target")
class P_MA_API UMASkillAction_ShatterFrozenTarget : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_ShatterFrozenTarget() { SupportedModuleTypes = EMASkillModuleType::Module; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Shatter", meta=(ClampMin="0.0"))
	float AdditionalDamageMultiplier = 2.f;

	UPROPERTY(EditDefaultsOnly, Category="Shatter", meta=(Categories="GameplayCue.Hit"))
	FGameplayTagContainer TargetGameplayCueTags;
};
