#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MASkillGameplayEventBinding.generated.h"

class UMASkillAction;
class UMASkillRuntimeScope;

USTRUCT(BlueprintType)
struct P_MA_API FMASkillGameplayEventBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Event")
	FGameplayTag EventTag;

	UPROPERTY(EditDefaultsOnly, Category="Event")
	bool bUseLocalBinding = false;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Action")
	TObjectPtr<UMASkillAction> Action;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillRuntimeScope> RuntimeScope = nullptr;
};


