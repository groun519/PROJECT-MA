#include "Player/Components/MACurrencyComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAPlayerAttributeSet.h"

UMACurrencyComponent::UMACurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UMACurrencyComponent::GetCoin() const
{
	bool bFound = false;
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	return AbilitySystemComponent
		? AbilitySystemComponent->GetGameplayAttributeValue(UMAPlayerAttributeSet::GetGoldAttribute(), bFound)
		: 0.f;
}

bool UMACurrencyComponent::HasCoin(float Amount) const
{
	return Amount <= 0.f || GetCoin() >= Amount;
}

bool UMACurrencyComponent::TrySpendCoin(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (Amount < 0.f || !AbilitySystemComponent) return false;

	bool bFound = false;
	if (AbilitySystemComponent->GetGameplayAttributeValue(UMAPlayerAttributeSet::GetGoldAttribute(), bFound) < Amount) return false;
	AbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, -Amount);
	return true;
}

void UMACurrencyComponent::AddCoin(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (Amount <= 0.f || !AbilitySystemComponent) return;

	AbilitySystemComponent->ApplyModToAttribute(UMAPlayerAttributeSet::GetGoldAttribute(), EGameplayModOp::Additive, Amount);
}
