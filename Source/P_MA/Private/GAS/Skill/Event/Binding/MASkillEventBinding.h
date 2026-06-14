#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "GameplayTagContainer.h"
#include "MASkillEventBinding.generated.h"

class UMASkillAction;
class UMASkillModuleInstance;

USTRUCT(BlueprintType)
struct P_MA_API FMASkillEventBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Event")
	FGameplayTag EventTag;

	UPROPERTY(EditDefaultsOnly, Category="Event")
	EMASkillEventBindingScope BindingScope = EMASkillEventBindingScope::Skill;

	UPROPERTY(meta=(DeprecatedProperty, DeprecationMessage="Use BindingScope."))
	bool bUseLocalBinding = false;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Action")
	TObjectPtr<UMASkillAction> Action;

	UPROPERTY(Transient)
	FMASkillScopes BindingScopes;

	bool TryResolveScopes(const FMASkillScopes& SourceScopes, FMASkillScopes& OutScopes) const;
};


