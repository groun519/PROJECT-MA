#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "MASkillModuleGameplayEffectAddon.generated.h"

class UAbilitySystemComponent;
struct FMASkillPayloadStore;

UENUM(BlueprintType)
enum class EMASkillModuleEffectMagnitudeType : uint8
{
	Snapshot,
	AttributeBased
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillModuleGameplayModifier
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Modifier")
	FGameplayAttribute Attribute;

	UPROPERTY(EditDefaultsOnly, Category="Modifier")
	TEnumAsByte<EGameplayModOp::Type> ModifierOp = EGameplayModOp::Additive;
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillModuleGameplayEffectConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Magnitude", meta=(TitleProperty="Attribute"))
	TArray<FMASkillModuleGameplayModifier> Modifiers;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Magnitude")
	float BaseMagnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Magnitude")
	EMASkillModuleEffectMagnitudeType MagnitudeType = EMASkillModuleEffectMagnitudeType::Snapshot;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Magnitude", meta=(EditCondition="MagnitudeType == EMASkillModuleEffectMagnitudeType::Snapshot", EditConditionHides))
	TArray<FMAAttributeCoefficient> MagnitudeCoefficients;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Magnitude", meta=(EditCondition="MagnitudeType == EMASkillModuleEffectMagnitudeType::AttributeBased", EditConditionHides))
	FGameplayAttribute MagnitudeAttribute;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Magnitude", meta=(EditCondition="MagnitudeType == EMASkillModuleEffectMagnitudeType::AttributeBased", EditConditionHides))
	float MagnitudeAttributeCoefficient = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect", meta=(Categories="OwnedTagsCategory"))
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Periodic", meta=(ClampMin="0.0", UIMin="0.0"))
	float Period = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Periodic", meta=(EditCondition="Period > 0.0", EditConditionHides))
	bool bExecutePeriodicEffectOnApplication = true;

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect|Gameplay Cue", meta=(Categories="GameplayCue"))
	FGameplayTagContainer GameplayCueTags;

	const UGameplayEffect* GetEffectDefinition() const { return EffectDefinition; }

private:
#if WITH_EDITOR
	void RebuildEffectDefinition(UObject& Outer);
#endif
	float ResolveMagnitude(
		const UAbilitySystemComponent& AbilitySystemComponent,
		const FMASkillPayloadStore& PayloadStore) const;

	/** Serialized GAS definition generated from the authored addon data. */
	UPROPERTY(Instanced)
	TObjectPtr<UGameplayEffect> EffectDefinition;

	friend class UMASkillModuleGameplayEffectAddon;
};

/** Defines self-target Infinite GameplayEffects owned by an assembly-active module. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillModuleGameplayEffectAddon : public UMASkillModuleAddon
{
	GENERATED_BODY()

public:
	UMASkillModuleGameplayEffectAddon()
	{
		SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub;
	}

	virtual void BindModule(UMASkillModuleInstance& ModuleInstance) const override;
	virtual void UnbindModule(UMASkillModuleInstance& ModuleInstance) const override;

#if WITH_EDITOR
	virtual void BuildGeneratedData() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	virtual UMASkillModuleAddon* AssembleInto(
		UObject& ResultOuter,
		UMASkillModuleAddon* ResultAddon,
		EMASkillAddonAssemblyStage Stage,
		const FMASkillScopes& SourceScopes) const override;
	void SetEffectsActive(UMASkillModuleInstance& ModuleInstance, bool bActive) const;
#if WITH_EDITOR
	void RebuildEffectDefinitions();
#endif

	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TArray<FMASkillModuleGameplayEffectConfig> GameplayEffects;
};
