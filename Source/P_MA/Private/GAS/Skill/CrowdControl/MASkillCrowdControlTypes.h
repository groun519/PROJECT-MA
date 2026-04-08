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

USTRUCT()
struct P_MA_API FResolvedCrowdControlEffect
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayEffectSpecHandle SpecHandle;

	UPROPERTY(Transient)
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;
};
