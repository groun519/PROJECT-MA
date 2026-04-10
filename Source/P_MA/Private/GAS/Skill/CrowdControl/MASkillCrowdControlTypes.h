#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "MASkillCrowdControlTypes.generated.h"

UENUM(BlueprintType)
enum class EMASkillCrowdControlSourceType : uint8
{
	Instigator,
	Center
};

USTRUCT(BlueprintType)
struct P_MA_API FMASkillCrowdControlGrantedStateRule
{
	GENERATED_BODY()

	FMASkillCrowdControlGrantedStateRule() = default;

	FMASkillCrowdControlGrantedStateRule(bool bInBlockMove, bool bInLockRotation, bool bInBlockAbility)
		: bBlockMove(bInBlockMove)
		, bLockRotation(bInLockRotation)
		, bBlockAbility(bInBlockAbility)
	{
	}

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl")
	bool bBlockMove = false;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl")
	bool bLockRotation = false;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl")
	bool bBlockAbility = false;

	bool HasAny() const
	{
		return bBlockMove || bLockRotation || bBlockAbility;
	}
};

USTRUCT()
struct P_MA_API FMASkillCrowdControlPolicy
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayTag CrowdControlTag;

	UPROPERTY(Transient)
	FGameplayTagContainer GrantedStateTags;

	UPROPERTY(Transient)
	float Magnitude = 0.f;

	UPROPERTY(Transient)
	float Duration = 0.f;

	UPROPERTY(Transient)
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;

	bool IsValid() const
	{
		return CrowdControlTag.IsValid() && Duration > 0.f;
	}
};

USTRUCT()
struct P_MA_API FResolvedCrowdControlEffect
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayEffectSpecHandle SpecHandle;

	UPROPERTY(Transient)
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;
};
