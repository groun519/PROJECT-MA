#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffectTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadStructBase.h"
#include "GameplayEffectTypes.h"
#include "MASkillDamageTypes.generated.h"

class UMASkillAbility;

UENUM(BlueprintType)
enum class EMASkillDamageApplicationMode : uint8
{
	Instant,
	DamageOverTime
};

UENUM(BlueprintType)
enum class EMASkillTargetRelationMergeOp : uint8
{
	None,
	Add,
	Remove,
	Replace
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillTargetRelationModifier
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	EMASkillTargetRelationMergeOp Operation = EMASkillTargetRelationMergeOp::None;

	UPROPERTY(EditDefaultsOnly, Category="Targeting", meta=(Bitmask, BitmaskEnum="/Script/P_MA.EMATargetRelation"))
	int32 RelationMask = MATargetRelation::ToMask(EMATargetRelation::None);

	void ApplyTo(int32& InOutMask) const
	{
		switch (Operation)
		{
		case EMASkillTargetRelationMergeOp::None:
			return;
		case EMASkillTargetRelationMergeOp::Add:
			InOutMask |= RelationMask;
			return;
		case EMASkillTargetRelationMergeOp::Remove:
			InOutMask &= ~RelationMask;
			return;
		case EMASkillTargetRelationMergeOp::Replace:
			InOutMask = RelationMask;
			return;
		default:
			return;
		}
	}

	bool HasOverride() const
	{
		return Operation != EMASkillTargetRelationMergeOp::None;
	}
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillDamageOverTimeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Damage|DoT", meta=(ClampMin="0.01"))
	float Duration = 3.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage|DoT", meta=(ClampMin="1"))
	int32 TickCount = 3;
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillDamageConfig : public FMASkillPayloadStructBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="DamageType"))
	FGameplayTag DamageTypeTag = FGameplayTag::RequestGameplayTag(TEXT("DamageType.Damage"));

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TArray<FMADamageAttributeCoefficient> AttributeCoefficients;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	EMASkillDamageApplicationMode ApplicationMode = EMASkillDamageApplicationMode::Instant;

	UPROPERTY(EditDefaultsOnly, Category="Damage|DoT", meta=(EditCondition="ApplicationMode == EMASkillDamageApplicationMode::DamageOverTime", EditConditionHides))
	FMASkillDamageOverTimeConfig DamageOverTime;

	UPROPERTY(EditDefaultsOnly, Category="Targeting", meta=(Bitmask, BitmaskEnum="/Script/P_MA.EMATargetRelation"))
	int32 TargetRelationMask = MATargetRelation::GetDefaultMask();

	UPROPERTY(EditDefaultsOnly, Instanced, Category="StatusEffect")
	TArray<TObjectPtr<UMASkillStatusEffect>> StatusEffects;

	UPROPERTY(EditDefaultsOnly, Category="GameplayCue", meta=(Categories="GameplayCue.Hit"))
	FGameplayTagContainer TargetGameplayCueTags;

	void Append(const FMASkillDamageConfig& Other)
	{
		BaseDamage += Other.BaseDamage;
		if (Other.DamageTypeTag.IsValid())
		{
			DamageTypeTag = Other.DamageTypeTag;
		}
		AttributeCoefficients.Append(Other.AttributeCoefficients);
		if (Other.ApplicationMode == EMASkillDamageApplicationMode::DamageOverTime)
		{
			ApplicationMode = Other.ApplicationMode;
			DamageOverTime = Other.DamageOverTime;
		}
		StatusEffects.Append(Other.StatusEffects);
		TargetGameplayCueTags.AppendTags(Other.TargetGameplayCueTags);
	}

	bool HasValues() const
	{
		if (!FMath::IsNearlyZero(BaseDamage)) return true;

		for (const FMADamageAttributeCoefficient& Coefficient : AttributeCoefficients)
		{
			if (!FMath::IsNearlyZero(Coefficient.Coefficient)) return true;
		}

		return StatusEffects.Num() > 0 || !TargetGameplayCueTags.IsEmpty();
	}

};

USTRUCT()
struct P_MA_API FResolvedSkillHitEffects
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 TargetRelationMask = MATargetRelation::ToMask(EMATargetRelation::None);

	UPROPERTY(Transient)
	FGameplayEffectSpecHandle DamageSpec;

	UPROPERTY(Transient)
	TArray<FResolvedStatusEffect> StatusEffects;

	UPROPERTY(Transient)
	FGameplayTagContainer TargetGameplayCueTags;
};
