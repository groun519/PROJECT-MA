#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "MASkillStatusEffectTypes.generated.h"

UENUM(BlueprintType)
enum class EMASkillStatusEffectSourceType : uint8
{
	Instigator,
	Center
};

UENUM()
enum class EMASkillStatusEffectStrengthPolicy : uint8
{
	None,
	LargerMagnitudeStronger,
	SmallerMagnitudeStronger
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillStatusEffectGrantedStateRule
{
	GENERATED_BODY()

	FMASkillStatusEffectGrantedStateRule() = default;

	FMASkillStatusEffectGrantedStateRule(bool bInBlockMove, bool bInLockRotation, bool bInBlockAbility)
		: bBlockMove(bInBlockMove)
		, bLockRotation(bInLockRotation)
		, bBlockAbility(bInBlockAbility)
	{
	}

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	bool bBlockMove = false;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	bool bLockRotation = false;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	bool bBlockAbility = false;

	bool HasAny() const
	{
		return bBlockMove || bLockRotation || bBlockAbility;
	}
};

USTRUCT()
struct P_MA_API FMASkillStatusEffectPolicy
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayTag StatusEffectTag;

	UPROPERTY(Transient)
	FGameplayTagContainer GrantedStateTags;

	UPROPERTY(Transient)
	float Magnitude = 0.f;

	UPROPERTY(Transient)
	float Duration = 0.f;

	UPROPERTY(Transient)
	EMASkillStatusEffectSourceType SourceType = EMASkillStatusEffectSourceType::Instigator;

	bool IsValid() const
	{
		return StatusEffectTag.IsValid() && Duration > 0.f;
	}
};

USTRUCT()
struct P_MA_API FResolvedStatusEffect
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayEffectSpecHandle SpecHandle;

	UPROPERTY(Transient)
	EMASkillStatusEffectSourceType SourceType = EMASkillStatusEffectSourceType::Instigator;

	UPROPERTY(Transient)
	EMASkillStatusEffectStrengthPolicy StrengthPolicy = EMASkillStatusEffectStrengthPolicy::None;

	UPROPERTY(Transient)
	float StrengthMagnitude = 0.f;
};
