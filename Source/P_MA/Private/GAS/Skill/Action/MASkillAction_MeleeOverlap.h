#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_MeleeOverlap.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Melee Overlap From Event Data")
class P_MA_API UMASkillAction_MeleeOverlap : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_MeleeOverlap() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;
};
