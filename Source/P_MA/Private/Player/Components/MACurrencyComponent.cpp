#include "Player/Components/MACurrencyComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"

UMACurrencyComponent::UMACurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UMACurrencyComponent::GetCoin() const
{
	bool bFound = false;
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	return AbilitySystemComponent
		? AbilitySystemComponent->GetGameplayAttributeValue(UMAAttributeSet::GetCoinAttribute(), bFound)
		: 0.f;
}

bool UMACurrencyComponent::TrySpendCoin(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (Amount < 0.f || !AbilitySystemComponent) return false;

	bool bFound = false;
	if (AbilitySystemComponent->GetGameplayAttributeValue(UMAAttributeSet::GetCoinAttribute(), bFound) < Amount) return false;
	AbilitySystemComponent->ApplyModToAttribute(UMAAttributeSet::GetCoinAttribute(), EGameplayModOp::Additive, -Amount);
	return true;
}

void UMACurrencyComponent::AddCoin(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (Amount <= 0.f || !AbilitySystemComponent) return;

	AbilitySystemComponent->ApplyModToAttribute(UMAAttributeSet::GetCoinAttribute(), EGameplayModOp::Additive, Amount);
}
