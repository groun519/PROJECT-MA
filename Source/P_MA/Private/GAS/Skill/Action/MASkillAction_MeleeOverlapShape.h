#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_MeleeOverlapShape.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Melee Overlap Shape")
class P_MA_API UMASkillAction_MeleeOverlapShape : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_MeleeOverlapShape();
	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Action", meta=(ShowOnlyInnerProperties))
	FMASkillAreaShape Config;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;
};
