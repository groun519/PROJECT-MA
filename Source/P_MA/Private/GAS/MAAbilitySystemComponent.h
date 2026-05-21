#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "PA_AbilitySystemGenerics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAAbilitySystemComponent.generated.h"

UCLASS()
class UMAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMAAbilitySystemComponent();
	void InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyFullStatEffect();
	void TryActivateAbilitiesByInputID(EMAAbilityInputID InputID);
	const UPA_AbilitySystemGenerics* GetSystemGenerics() const {return AbilitySystemGenerics;};

	UPROPERTY(Transient)
	FGameplayTagContainer AppliedBaseTags;
	
private:
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	UPA_AbilitySystemGenerics* AbilitySystemGenerics;
};
