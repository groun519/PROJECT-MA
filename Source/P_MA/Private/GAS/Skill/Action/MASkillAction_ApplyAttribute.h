#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_ApplyAttribute.generated.h"

UENUM(BlueprintType)
enum class EMASkillAttributeApplyTarget : uint8
{
	Self,
	TargetPayload
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="Apply Attribute")
class P_MA_API UMASkillAction_ApplyAttribute : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;
	virtual void Execute(UMASkillAbility& OwnerAbility, const FMASkillEvent& Event, const FMASkillScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Target")
	EMASkillAttributeApplyTarget Target = EMASkillAttributeApplyTarget::Self;

	UPROPERTY(EditDefaultsOnly, Category="Target", meta=(Categories="Data", EditCondition="Target == EMASkillAttributeApplyTarget::TargetPayload", EditConditionHides))
	FGameplayTag TargetPayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Attribute")
	FGameplayAttribute Attribute;

	UPROPERTY(EditDefaultsOnly, Category="Attribute")
	TEnumAsByte<EGameplayModOp::Type> Operation = EGameplayModOp::Additive;

	UPROPERTY(EditDefaultsOnly, Category="Attribute")
	float BaseValue = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Attribute")
	float EventMagnitudeCoefficient = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Attribute")
	TArray<FMAAttributeCoefficient> AttributeCoefficients;
};
