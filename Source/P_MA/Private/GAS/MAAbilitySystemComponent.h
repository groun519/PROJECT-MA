#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "MAAbilitySystemComponent.generated.h"

struct FGameplayEffectModCallbackData;
class UAnimMontage;
class UGameplayAbility;

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
	float PlayMontageWithBlendIn(
		UGameplayAbility* AnimatingAbility,
		FGameplayAbilityActivationInfo ActivationInfo,
		UAnimMontage* Montage,
		float PlayRate,
		FName StartSectionName,
		float BlendInTime);
	virtual float PlayMontageSimulated(
		UAnimMontage* Montage,
		float PlayRate,
		FName StartSectionName = NAME_None) override;

	UPROPERTY(Transient)
	FGameplayTagContainer AppliedBaseTags;
	
private:
	void GiveInitialAbilities();
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ShowDamageText(const FGameplayEffectModCallbackData& Data, bool bIsIncoming) const;
	void ApplyMontageBlendIn(UAnimMontage* Montage, float PlayRate, float BlendInTime) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> BasicAbilities;
};
