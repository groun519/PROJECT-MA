#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl_State.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl_Impulse.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl_Airborne.h"
#include "GAS/Skill/CrowdControl/MASkillStatusEffect_Attribute.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlTypes.h"
#include "GAS/Skill/Payload/MASkillPayloadStructBase.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayEffectTypes.h"
#include "MASkillDamageConfig.generated.h"

class UMASkillAbility;

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
struct P_MA_API FMASkillDamageConfig : public FMASkillPayloadStructBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float FinalDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TArray<FMADamageAttributeCoefficient> AttributeCoefficients;

	UPROPERTY(EditDefaultsOnly, Category="Targeting", meta=(Bitmask, BitmaskEnum="/Script/P_MA.EMATargetRelation"))
	int32 TargetRelationMask = MATargetRelation::GetDefaultMask();

	UPROPERTY(EditDefaultsOnly, Instanced, Category="CrowdControl")
	TArray<TObjectPtr<UMASkillCrowdControl>> CrowdControls;

	void Append(const FMASkillDamageConfig& Other)
	{
		BaseDamage += Other.BaseDamage;
		FinalDamageMultiplier *= Other.FinalDamageMultiplier;
		AttributeCoefficients.Append(Other.AttributeCoefficients);
		CrowdControls.Append(Other.CrowdControls);
	}

	bool HasValues() const
	{
		if (!FMath::IsNearlyZero(BaseDamage)) return true;
		if (!FMath::IsNearlyEqual(FinalDamageMultiplier, 1.f)) return true;

		for (const FMADamageAttributeCoefficient& Coefficient : AttributeCoefficients)
		{
			if (!FMath::IsNearlyZero(Coefficient.Coefficient)) return true;
		}

		return CrowdControls.Num() > 0;
	}

	FMADamageExecutionConfig ToExecutionConfig() const
	{
		FMADamageExecutionConfig Result;
		Result.BaseDamage = BaseDamage;
		Result.FinalDamageMultiplier = FinalDamageMultiplier;
		Result.AttributeCoefficients = AttributeCoefficients;
		return Result;
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
	TArray<FResolvedCrowdControlEffect> CrowdControlEffects;
};

namespace MASkillResolvedHitEffects
{
	P_MA_API FResolvedSkillHitEffects BuildResolvedHitEffects(UMASkillAbility& OwnerAbility, const FMASkillDamageConfig& DamageConfig);
}
