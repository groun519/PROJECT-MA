#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "PA_AbilitySystemGenerics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAAbilitySystemComponent.generated.h"

struct FGameplayEffectModCallbackData;

struct FMADamageAppliedEvent
{
	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<AActor> TargetActor;
	FHitResult HitResult;
	float DisplayMagnitude = 0.f;
	FGameplayTag DamageTypeTag;
	bool bIsCriticalHit = false;
};

UCLASS()
class UMAAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UMAAbilitySystemComponent();
	void InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyFullStatEffect();
	void ApplyReviveStatEffect();
	void NotifyDamageAppliedFromGameplayEffect(const FGameplayEffectModCallbackData& Data);
	const UPA_AbilitySystemGenerics* GetSystemGenerics() const {return AbilitySystemGenerics;};

	UPROPERTY(Transient)
	FGameplayTagContainer AppliedBaseTags;
	
private:
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ShowDamageText(const FMADamageAppliedEvent& DamageAppliedEvent, bool bIsIncoming) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	UPA_AbilitySystemGenerics* AbilitySystemGenerics;
};
