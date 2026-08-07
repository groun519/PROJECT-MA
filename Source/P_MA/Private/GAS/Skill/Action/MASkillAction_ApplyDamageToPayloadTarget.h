#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_ApplyDamageToPayloadTarget.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Apply Damage To Payload Target")
class P_MA_API UMASkillAction_ApplyDamageToPayloadTarget : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_ApplyDamageToPayloadTarget() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Target", meta=(Categories="Data"))
	FGameplayTag TargetPayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Damage|Multiplier")
	float BaseDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage|Multiplier")
	TArray<FMAAttributeCoefficient> DamageMultiplierCoefficients;
};
