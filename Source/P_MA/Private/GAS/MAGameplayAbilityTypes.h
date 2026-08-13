#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "MAGameplayAbilityTypes.generated.h"

struct FMAGameplayEffectContext;
struct FMASkillPayloadAccess;
class UAbilitySystemComponent;
class UMASkillModuleInstance;

UENUM(BlueprintType)
enum class EMADamageCriticalResult : uint8
{
	None,
	Critical,
	ReverseCritical
};

USTRUCT()
struct FMAGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	EMADamageCriticalResult GetCriticalResult() const { return CriticalResult; }
	void SetCriticalResult(EMADamageCriticalResult InCriticalResult) { CriticalResult = InCriticalResult; }
	const FGameplayTag& GetDamageTypeTag() const { return DamageTypeTag; }
	void SetDamageTypeTag(const FGameplayTag& InDamageTypeTag) { DamageTypeTag = InDamageTypeTag; }
	float GetDisplayMagnitude() const { return DisplayMagnitude; }
	void SetDisplayMagnitude(float InDisplayMagnitude) { DisplayMagnitude = InDisplayMagnitude; }
	UMASkillModuleInstance* GetSkillScope() const;
	void SetSkillScope(UMASkillModuleInstance* InSkillScope);
	virtual UScriptStruct* GetScriptStruct() const override {return StaticStruct();}
	virtual FMAGameplayEffectContext* Duplicate() const override
	{
		FMAGameplayEffectContext* NewContext = new FMAGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

protected:
	UPROPERTY()
	EMADamageCriticalResult CriticalResult = EMADamageCriticalResult::None;

	UPROPERTY()
	FGameplayTag DamageTypeTag;

	UPROPERTY()
	float DisplayMagnitude = 0.f;

	// The exact assembled skill that produced this effect; server-only routing data.
	UPROPERTY(Transient)
	TWeakObjectPtr<UMASkillModuleInstance> SkillScope;
};

template<>
struct TStructOpsTypeTraits<FMAGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMAGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

USTRUCT(BlueprintType)
struct FGenericDamageEffectDef
{
	GENERATED_BODY()

	FGenericDamageEffectDef();
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditAnywhere)
	FVector PushVelocity;
};

UENUM(BlueprintType)
enum class EMACoefficientSource : uint8
{
	Source,
	Target,
	Payload
};

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EMATargetRelation : uint8
{
	None     = 0 UMETA(Hidden),
	Friendly = 1 << 0,
	Hostile  = 1 << 1,
	Neutral  = 1 << 2,
	Self     = 1 << 3
};
ENUM_CLASS_FLAGS(EMATargetRelation);

namespace MATargetRelation
{
	FORCEINLINE int32 ToMask(const EMATargetRelation Relation)
	{
		return static_cast<int32>(Relation);
	}

	FORCEINLINE int32 ToMask(const ETeamAttitude::Type TeamAttitude)
	{
		switch (TeamAttitude)
		{
		case ETeamAttitude::Friendly:
			return ToMask(EMATargetRelation::Friendly);
		case ETeamAttitude::Hostile:
			return ToMask(EMATargetRelation::Hostile);
		case ETeamAttitude::Neutral:
			return ToMask(EMATargetRelation::Neutral);
		default:
			return ToMask(EMATargetRelation::None);
		}
	}

	FORCEINLINE int32 GetDefaultMask()
	{
		return ToMask(EMATargetRelation::Hostile);
	}

	FORCEINLINE bool IncludesSelf(const int32 AllowedRelationMask)
	{
		return (AllowedRelationMask & ToMask(EMATargetRelation::Self)) != 0;
	}

	FORCEINLINE bool IsSelfTarget(const AActor* SourceActor, const AActor* TargetActor)
	{
		return SourceActor && TargetActor && SourceActor == TargetActor;
	}

	FORCEINLINE bool MatchesMask(const int32 AllowedRelationMask, const ETeamAttitude::Type TeamAttitude)
	{
		return (AllowedRelationMask & ToMask(TeamAttitude)) != 0;
	}

	FORCEINLINE bool MatchesTarget(const int32 AllowedRelationMask, const AActor* SourceActor, const AActor* TargetActor, const ETeamAttitude::Type TeamAttitude)
	{
		if (IsSelfTarget(SourceActor, TargetActor))
		{
			return IncludesSelf(AllowedRelationMask);
		}

		return MatchesMask(AllowedRelationMask, TeamAttitude);
	}
}

UENUM(BlueprintType)
enum class EMAPayloadCalculationType : uint8
{
	Linear,
	DiminishingGrowth
};

USTRUCT(BlueprintType)
struct FMAPayloadCalculation
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Calculation")
	EMAPayloadCalculationType Type = EMAPayloadCalculationType::Linear;

	/** Input value that produces half of MaxValue. */
	UPROPERTY(EditDefaultsOnly, Category="Calculation",
		meta=(EditCondition="Type == EMAPayloadCalculationType::DiminishingGrowth", EditConditionHides, ClampMin="0.0001", UIMin="0.0001"))
	float HalfValue = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Calculation",
		meta=(EditCondition="Type == EMAPayloadCalculationType::DiminishingGrowth", EditConditionHides))
	float MaxValue = 1.f;

	float Calculate(float Value) const;
};

USTRUCT(BlueprintType)
struct FMAAttributeCoefficient
{
	GENERATED_BODY()

	FMAAttributeCoefficient();

	UPROPERTY(EditDefaultsOnly, Category="Coefficient")
	EMACoefficientSource Source = EMACoefficientSource::Source;

	UPROPERTY(EditDefaultsOnly, Category="Coefficient", meta=(DisplayName="Attribute", EditCondition="Source != EMACoefficientSource::Payload", EditConditionHides))
	FGameplayAttribute GameplayAttribute;

	UPROPERTY(EditDefaultsOnly, Category="Coefficient", meta=(Categories="Data", EditCondition="Source == EMACoefficientSource::Payload", EditConditionHides))
	FGameplayTag PayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Coefficient", meta=(EditCondition="Source == EMACoefficientSource::Payload", EditConditionHides))
	FMAPayloadCalculation PayloadCalculation;

	UPROPERTY(EditDefaultsOnly, Category="Coefficient")
	float Coefficient = 0.f;

	float ResolvePayloadContribution(const FMASkillPayloadAccess& Payloads) const;

	float ResolveValue(
		const UAbilitySystemComponent& SourceASC,
		const UAbilitySystemComponent& TargetASC,
		const FMASkillPayloadAccess& Payloads) const;
};

USTRUCT(BlueprintType)
struct FMADamageExecutionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	FGameplayTag DamageTypeTag = FGameplayTag::RequestGameplayTag(TEXT("DamageType.Damage"));

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TArray<FMAAttributeCoefficient> AttributeCoefficients;

	void Append(const FMADamageExecutionConfig& Other)
	{
		BaseDamage += Other.BaseDamage;
		if (Other.DamageTypeTag.IsValid())
		{
			DamageTypeTag = Other.DamageTypeTag;
		}
		AttributeCoefficients.Append(Other.AttributeCoefficients);
	}

	bool HasValues() const
	{
		if (!FMath::IsNearlyZero(BaseDamage)) return true;

		for (const FMAAttributeCoefficient& Coefficient : AttributeCoefficients)
		{
			if (!FMath::IsNearlyZero(Coefficient.Coefficient)
				&& Coefficient.GameplayAttribute.IsValid())
			{
				return true;
			}
		}

		return false;
	}
};

USTRUCT(BlueprintType)
struct FPlayerBaseStats : public FTableRowBase
{
	GENERATED_BODY()
FPlayerBaseStats();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseAttack;
	
	UPROPERTY(EditAnywhere)
	float BaseAttackSpeed;
	
	UPROPERTY(EditAnywhere)
	float BaseFocus;

	UPROPERTY(EditAnywhere)
	float BaseCriticalDamage;

	UPROPERTY(EditAnywhere)
	float BaseReverseCriticalDamage;
	
	UPROPERTY(EditAnywhere)
	float BaseAreaRangeScale;

	UPROPERTY(EditAnywhere)
	float BaseProjectileRangeScale;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
	
	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseArmorPenetration;
	
	UPROPERTY(EditAnywhere)
	float BaseCoin;
};

USTRUCT(BlueprintType)
struct FMonsterBaseStats : public FTableRowBase
{
	GENERATED_BODY()
	FMonsterBaseStats();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseAttack;
	
	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;

	UPROPERTY(EditAnywhere)
	float BaseAttackSpeed;

	UPROPERTY(EditAnywhere)
	float BaseAreaRangeScale;

	UPROPERTY(EditAnywhere)
	float BaseProjectileRangeScale;

	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseArmorPenetration;

	UPROPERTY(EditAnywhere, meta=(Categories="Stats.Immunity"))
	FGameplayTagContainer BaseImmunityTags;
};
