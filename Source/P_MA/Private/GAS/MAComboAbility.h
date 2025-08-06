#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MAComboAbility.generated.h"

UCLASS()
class P_MA_API UMAComboAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMAComboAbility();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer AbilityTagsEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer CancelAbilitiesWithTagEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer BlockAbilitiesWithTagEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer ActivationOwnedTagsEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer ActivationRequiredTagsEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer ActivationBlockedTagsEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer SourceRequiredTagsEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer SourceBlockedTagsEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer TargetRequiredTagsEditable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer TargetBlockedTagsEditable;
};