#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "MAGameplayEffect_StatusEffectAttribute.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMAGameplayEffect_StatusEffectAttribute : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_StatusEffectAttribute();
	void SetTargetAttribute(FGameplayAttribute InAttribute);
	void SetModifierOp(EGameplayModOp::Type InModifierOp);
	void SetMagnitude(float InMagnitude);

protected:
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	FGameplayAttribute TargetAttribute;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	TEnumAsByte<EGameplayModOp::Type> ModifierOp = EGameplayModOp::Multiplicitive;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	float Magnitude = 1.f;

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void RebuildModifiers();
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="GE StatusEffect Slow")
class P_MA_API UMAGameplayEffect_StatusEffectSlow : public UMAGameplayEffect_StatusEffectAttribute
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_StatusEffectSlow();
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, DisplayName="GE StatusEffect Haste")
class P_MA_API UMAGameplayEffect_StatusEffectHaste : public UMAGameplayEffect_StatusEffectAttribute
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_StatusEffectHaste();
};
