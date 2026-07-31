#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "MASkillAction_ApplyGameplayEffectToSelf.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Apply Gameplay Effect To Self")
class P_MA_API UMASkillAction_ApplyGameplayEffectToSelf : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_ApplyGameplayEffectToSelf() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Item | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="GameplayEffect", meta=(ClampMin="0.0"))
	float EffectLevel = 1.f;
};
