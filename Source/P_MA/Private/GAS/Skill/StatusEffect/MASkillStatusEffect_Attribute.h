#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"
#include "MASkillStatusEffect_Attribute.generated.h"

class UMAGameplayEffect_StatusEffectAttribute;

UCLASS(BlueprintType, DisplayName="SE Attribute")
class P_MA_API UMASkillStatusEffect_Attribute : public UMASkillStatusEffect
{
	GENERATED_BODY()

public:
	UMASkillStatusEffect_Attribute() = default;
	virtual bool BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedStatusEffect>& OutEffects) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Duration = 3.f;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Internal")
	TObjectPtr<UMAGameplayEffect_StatusEffectAttribute> EffectTemplate;

	UPROPERTY(EditDefaultsOnly, Category="Internal")
	FGameplayTagContainer GrantedTags;

	virtual void PrepareEffectTemplate() const {}
	virtual EMASkillStatusEffectStrengthPolicy GetStrengthPolicy() const { return EMASkillStatusEffectStrengthPolicy::None; }
	virtual float GetStrengthMagnitude() const { return 0.f; }
	virtual bool ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const override;
};

UCLASS(BlueprintType, DisplayName="SE Slow", HideCategories="Internal")
class P_MA_API UMASkillStatusEffect_Slow : public UMASkillStatusEffect_Attribute
{
	GENERATED_BODY()

public:
	UMASkillStatusEffect_Slow(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Magnitude = 0.8f;

	virtual void PrepareEffectTemplate() const override;
};

UCLASS(BlueprintType, DisplayName="SE Haste", HideCategories="Internal")
class P_MA_API UMASkillStatusEffect_Haste : public UMASkillStatusEffect_Attribute
{
	GENERATED_BODY()

public:
	UMASkillStatusEffect_Haste(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Magnitude = 1.2f;

	virtual void PrepareEffectTemplate() const override;
	virtual EMASkillStatusEffectStrengthPolicy GetStrengthPolicy() const override { return EMASkillStatusEffectStrengthPolicy::LargerMagnitudeStronger; }
	virtual float GetStrengthMagnitude() const override { return Magnitude; }
};
