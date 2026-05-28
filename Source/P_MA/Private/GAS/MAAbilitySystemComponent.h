#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "PA_AbilitySystemGenerics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAAbilitySystemComponent.generated.h"

struct FMADamageAppliedEvent
{
	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<AActor> TargetActor;
	FHitResult HitResult;
	float Amount = 0.f;
	FGameplayTag DamageTypeTag;
	bool bIsCriticalHit = false;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FMADamageAppliedSignature, const FMADamageAppliedEvent&);

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
	void NotifyDamageApplied(const FMADamageAppliedEvent& DamageEvent, bool bIsIncoming);
	FMADamageAppliedSignature& OnDamageApplied() { return DamageAppliedDelegate; }
	const UPA_AbilitySystemGenerics* GetSystemGenerics() const {return AbilitySystemGenerics;};

	UPROPERTY(Transient)
	FGameplayTagContainer AppliedBaseTags;
	
private:
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ShowDamageNumber(const FMADamageAppliedEvent& DamageEvent, bool bIsIncoming) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	UPA_AbilitySystemGenerics* AbilitySystemGenerics;

	FMADamageAppliedSignature DamageAppliedDelegate;
};
