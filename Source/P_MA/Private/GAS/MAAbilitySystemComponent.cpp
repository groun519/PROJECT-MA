// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"

UMAAbilitySystemComponent::UMAAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetHealthAttribute()).AddUObject(this, &UMAAbilitySystemComponent::HealthUpdated);
}

void UMAAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (const TSubclassOf<UGameplayEffect>& EffectClass : InitialEffects)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UMAAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (const TPair<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, (int32)AbilityPair.Key, nullptr));
	}

	for (const TPair<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, (int32)AbilityPair.Key, nullptr));
	}
}

void UMAAbilitySystemComponent::ApplyFullStatEffect()
{
	AuthApplyGameplayEffect(FullStatEffect);
}

void UMAAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

/** 
 * @brief 체력 변경 콜백.
 * @details 체력이 0 이하이면 서버 권한(Authority)을 확인하고 DeathEffect를 적용합니다.
 * 
 * @param ChangeData 변경된 Attribute 정보(이전/새 값).
 *
 * @note 서버에서만 상태 변화 적용.
 */
void UMAAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	/** [Exception Handling] **//**
	 * 1. GetOwner()
	 *		=>	find MACharacter(AActor*)
	 *			AActorComponent's Owner automatically link (Component's attaching Actor)
	 *			
	 * 2. (!GetOwner()) return;
	 *		=>	if MAAbilitySystemComponent not have Owner, return.
	 *			(if Character is Dead)
	 */
	if (!GetOwner()) return;
	
	if (ChangeData.NewValue <= 0 && GetOwner()->HasAuthority() && DeathEffect)
	{
		AuthApplyGameplayEffect(DeathEffect);
	}
}