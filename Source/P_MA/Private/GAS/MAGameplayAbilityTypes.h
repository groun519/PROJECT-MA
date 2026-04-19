#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayEffect.h"
#include "MAGameplayAbilityTypes.generated.h"

struct FMAGameplayEffectContext;

USTRUCT()
struct FMAGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	bool IsCriticalHit() const {return bIsCriticalHit;}
	void SetIsCriticalHit(bool bInIsCriticalHit) {bIsCriticalHit = bInIsCriticalHit;}
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
	bool bIsCriticalHit = false;
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

UENUM(BlueprintType)
enum class EMAAbilityInputID : uint8
{
	None				UMETA(DisplayName = "None"),

	Attack				UMETA(DisplayName = "Attack"),
	Skill1				UMETA(DisplayName = "Skill1"),
	Skill2				UMETA(DisplayName = "Skill2"),
	Skill3				UMETA(DisplayName = "Skill3"),
	Skill4				UMETA(DisplayName = "Skill4"),
	Ultimate			UMETA(DisplayName = "Ultimate"),
	
	Movement			UMETA(DisplayName = "Movement"),

	Confirm				UMETA(DisplayName = "Confirm"),
	Cancel				UMETA(DisplayName = "Cancel"),
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
enum class EMADamageAttribute : uint8
{
	Health,
	MaxHealth,
	Attack,
	MoveSpeed,
	AttackSpeed,
	Armor,
	ArmorPenetration,
	CriticalChance,
	CriticalDamage
};

UENUM(BlueprintType)
enum class EMADamageAttributeSide : uint8
{
	Source,
	Target
};

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EMATargetRelation : uint8
{
	None     = 0 UMETA(Hidden),
	Friendly = 1 << 0,
	Hostile  = 1 << 1,
	Neutral  = 1 << 2
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

	FORCEINLINE bool MatchesMask(const int32 AllowedRelationMask, const ETeamAttitude::Type TeamAttitude)
	{
		return (AllowedRelationMask & ToMask(TeamAttitude)) != 0;
	}
}

USTRUCT(BlueprintType)
struct FMADamageAttributeCoefficient
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	EMADamageAttributeSide Side = EMADamageAttributeSide::Source;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	EMADamageAttribute Attribute = EMADamageAttribute::Attack;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float Coefficient = 0.f;
};

USTRUCT(BlueprintType)
struct FMADamageExecutionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float BaseDamage = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float FinalDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TArray<FMADamageAttributeCoefficient> AttributeCoefficients;

	void Append(const FMADamageExecutionConfig& Other)
	{
		BaseDamage += Other.BaseDamage;
		FinalDamageMultiplier *= Other.FinalDamageMultiplier;
		AttributeCoefficients.Append(Other.AttributeCoefficients);
	}

	bool HasValues() const
	{
		if (!FMath::IsNearlyZero(BaseDamage)) return true;
		if (!FMath::IsNearlyEqual(FinalDamageMultiplier, 1.f)) return true;

		for (const FMADamageAttributeCoefficient& Coefficient : AttributeCoefficients)
		{
			if (!FMath::IsNearlyZero(Coefficient.Coefficient)) return true;
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
	float BaseDamageVariance;

	UPROPERTY(EditAnywhere)
	float BaseAttackSpeed;
	
	UPROPERTY(EditAnywhere)
	float BaseCriticalChance;

	UPROPERTY(EditAnywhere)
	float BaseCriticalDamage;
	
	UPROPERTY(EditAnywhere)
	float BaseAttackRange;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
	
	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseArmorPenetration;
	
	UPROPERTY(EditAnywhere)
	float BaseGold;
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
	float BaseDamageVariance;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;

	UPROPERTY(EditAnywhere)
	float BaseAttackSpeed;

	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseArmorPenetration;

	UPROPERTY(EditAnywhere)
	float BaseFuryMax;
	
	UPROPERTY(EditAnywhere, meta=(Categories="Stats.Immunity"))
	FGameplayTagContainer BaseImmunityTags;
};
