#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_ApplyStatusEffectToSelf.generated.h"

class UMASkillStatusEffect;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Apply StatusEffect To Self")
class P_MA_API UMASkillAction_ApplyStatusEffectToSelf : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_ApplyStatusEffectToSelf() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Instanced, Category="StatusEffect")
	TArray<TObjectPtr<UMASkillStatusEffect>> StatusEffects;
};
