#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_PatchDamagePayload.generated.h"

UENUM(BlueprintType)
enum class EMASkillGameplayCuePatchOp : uint8
{
	None,
	Append,
	Replace
};

UENUM(BlueprintType)
enum class EMASkillStatusEffectPatchOp : uint8
{
	None,
	Append,
	Replace
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Patch Damage Payload")
class P_MA_API UMASkillAction_PatchDamagePayload : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_PatchDamagePayload() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	bool bExactPayloadTagMatch = true;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	bool bOverrideDamageType = false;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="DamageType", EditCondition="bOverrideDamageType", EditConditionHides))
	FGameplayTag DamageTypeTag;

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	FMASkillTargetRelationModifier TargetRelationModifier;

	UPROPERTY(EditDefaultsOnly, Category="GameplayCue")
	EMASkillGameplayCuePatchOp GameplayCuePatchOp = EMASkillGameplayCuePatchOp::None;

	UPROPERTY(EditDefaultsOnly, Category="GameplayCue", meta=(Categories="GameplayCue.Hit", EditCondition="GameplayCuePatchOp != EMASkillGameplayCuePatchOp::None", EditConditionHides))
	FGameplayTagContainer TargetGameplayCueTags;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	EMASkillStatusEffectPatchOp StatusEffectPatchOp = EMASkillStatusEffectPatchOp::None;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="StatusEffect", meta=(EditCondition="StatusEffectPatchOp != EMASkillStatusEffectPatchOp::None", EditConditionHides))
	TArray<TObjectPtr<UMASkillStatusEffect>> StatusEffects;
};
