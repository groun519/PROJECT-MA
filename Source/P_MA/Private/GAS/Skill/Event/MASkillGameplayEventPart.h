#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MASkillGameplayEventPart.generated.h"

class UMASkillAction;

USTRUCT(BlueprintType)
struct P_MA_API FMASkillGameplayEventPart
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Event")
	FGameplayTag EventTag;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Action")
	TObjectPtr<UMASkillAction> Action;
};
