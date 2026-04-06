#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "GameplayTagContainer.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GenericTeamAgentInterface.h"
#include "MASkillDamageConfig.generated.h"

UENUM(BlueprintType)
enum class EMASkillCrowdControlSourceType : uint8
{
	Instigator,
	Center
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillCrowdControlEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(Categories="State"))
	FGameplayTag CrowdControlTag;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl")
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;

	bool HasValidData() const
	{
		if (!CrowdControlTag.IsValid()) return false;
		return Duration > 0.f;
	}
};

USTRUCT(BlueprintType)
struct P_MA_API FSkillCrowdControlConfigBase
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct P_MA_API FSkillCrowdControlStunConfig : public FSkillCrowdControlConfigBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;
};

USTRUCT(BlueprintType)
struct P_MA_API FSkillCrowdControlKnockbackConfig : public FSkillCrowdControlConfigBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl")
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;
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
struct P_MA_API FMASkillDamageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TArray<FMADamageAttributeCoefficient> AttributeCoefficients;

	UPROPERTY(EditDefaultsOnly, Category="Targeting", meta=(Bitmask, BitmaskEnum="/Script/P_MA.EMATargetRelation"))
	int32 TargetRelationMask = MATargetRelation::GetDefaultMask();

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(BaseStruct="/Script/P_MA.SkillCrowdControlConfigBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> CrowdControlConfigs;

	static bool TryResolveCrowdControlEntry(const FInstancedStruct& CrowdControlConfig, FMASkillCrowdControlEntry& OutEntry)
	{
		if (const FSkillCrowdControlStunConfig* StunConfig = CrowdControlConfig.GetPtr<FSkillCrowdControlStunConfig>())
		{
			OutEntry.CrowdControlTag = FGameplayTag::RequestGameplayTag(TEXT("State.Debuff.Stun"));
			OutEntry.Magnitude = 0.f;
			OutEntry.Duration = StunConfig->Duration;
			OutEntry.SourceType = EMASkillCrowdControlSourceType::Instigator;
			return OutEntry.HasValidData();
		}

		if (const FSkillCrowdControlKnockbackConfig* KnockbackConfig = CrowdControlConfig.GetPtr<FSkillCrowdControlKnockbackConfig>())
		{
			OutEntry.CrowdControlTag = FGameplayTag::RequestGameplayTag(TEXT("State.Debuff.Knockback"));
			OutEntry.Magnitude = KnockbackConfig->Magnitude;
			OutEntry.Duration = KnockbackConfig->Duration;
			OutEntry.SourceType = KnockbackConfig->SourceType;
			return OutEntry.HasValidData();
		}

		return false;
	}

	void GatherResolvedCrowdControlEntries(TArray<FMASkillCrowdControlEntry>& OutEntries) const
	{
		for (const FInstancedStruct& CrowdControlConfig : CrowdControlConfigs)
		{
			FMASkillCrowdControlEntry CrowdControlEntry;
			if (!TryResolveCrowdControlEntry(CrowdControlConfig, CrowdControlEntry)) continue;
			OutEntries.Add(CrowdControlEntry);
		}
	}

	void Append(const FMASkillDamageConfig& Other)
	{
		BaseDamage += Other.BaseDamage;
		AttributeCoefficients.Append(Other.AttributeCoefficients);
		CrowdControlConfigs.Append(Other.CrowdControlConfigs);
	}

	bool HasValues() const
	{
		if (!FMath::IsNearlyZero(BaseDamage)) return true;

		for (const FMADamageAttributeCoefficient& Coefficient : AttributeCoefficients)
		{
			if (!FMath::IsNearlyZero(Coefficient.Coefficient)) return true;
		}

		for (const FInstancedStruct& CrowdControlConfig : CrowdControlConfigs)
		{
			FMASkillCrowdControlEntry CrowdControlEntry;
			if (TryResolveCrowdControlEntry(CrowdControlConfig, CrowdControlEntry)) return true;
		}

		return false;
	}

	FMADamageExecutionConfig ToExecutionConfig() const
	{
		FMADamageExecutionConfig Result;
		Result.BaseDamage = BaseDamage;
		Result.AttributeCoefficients = AttributeCoefficients;
		return Result;
	}
};
