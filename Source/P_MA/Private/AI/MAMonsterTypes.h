#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MAMonsterTypes.generated.h"

class UMASkillDefinition;

USTRUCT(BlueprintType)
struct FMonsterSkillSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FGameplayTag SlotTag;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float SelectionWeight = 1.f;

	UPROPERTY(EditAnywhere, meta=(ClampMin = "0.0"))
	float UseDistance = 300.f;

	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UMASkillDefinition>> Definitions;
};

USTRUCT(BlueprintType)
struct FMonsterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(ClampMin = "1", ClampMax = "10000"))
	int32 Gold = 0;

	UPROPERTY(EditAnywhere)
	TArray<FMonsterSkillSlotData> SkillSlots;
};
