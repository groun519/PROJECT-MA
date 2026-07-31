#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_MultiplyFinalDamage.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Multiply Final Damage")
class P_MA_API UMASkillAction_MultiplyFinalDamage : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_MultiplyFinalDamage() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float Multiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayTag MultiplierPayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float PayloadBaseMultiplier = 0.f;
};
