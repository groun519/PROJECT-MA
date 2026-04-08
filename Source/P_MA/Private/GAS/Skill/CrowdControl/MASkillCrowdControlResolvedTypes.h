#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GAS/Skill/MASkillDamageConfig.h"
#include "MASkillCrowdControlResolvedTypes.generated.h"

USTRUCT()
struct P_MA_API FResolvedCrowdControlEffect
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FGameplayEffectSpecHandle SpecHandle;

	UPROPERTY(Transient)
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;
};
