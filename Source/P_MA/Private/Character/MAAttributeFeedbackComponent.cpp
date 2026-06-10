#include "Character/MAAttributeFeedbackComponent.h"

#include "AbilitySystemInterface.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GameFramework/Pawn.h"
#include "Player/MAPlayerController.h"
#include "Setting/MAGameSettings.h"

void UMAAttributeFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	IAbilitySystemInterface* AbilityOwner = Cast<IAbilitySystemInterface>(GetOwner());
	AbilitySystemComponent = AbilityOwner ? Cast<UMAAbilitySystemComponent>(AbilityOwner->GetAbilitySystemComponent()) : nullptr;
	if (UMAAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetShieldAttribute()).AddUObject(this, &UMAAttributeFeedbackComponent::HandleShieldChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetCoinAttribute()).AddUObject(this, &UMAAttributeFeedbackComponent::HandleCoinChanged);
	}
}

void UMAAttributeFeedbackComponent::HandleShieldChanged(const FOnAttributeChangeData& Data) const
{
	const float Delta = Data.NewValue - Data.OldValue;
	if (Delta <= 0.f) return;

	const FMADamageTextStyle Style = UMAGameSettings::Get()->ShieldAttributeFeedbackTextStyle;
	ShowPositiveAttributeDelta(Delta, NSLOCTEXT("AttributeFeedback", "ShieldSuffix", ""), Style.ZOffset, Style.Color, Style.OutlineColor);
}

void UMAAttributeFeedbackComponent::HandleCoinChanged(const FOnAttributeChangeData& Data) const
{
	const float Delta = Data.NewValue - Data.OldValue;
	if (Delta <= 0.f) return;

	const FMADamageTextStyle Style = UMAGameSettings::Get()->CoinAttributeFeedbackTextStyle;
	ShowPositiveAttributeDelta(Delta, NSLOCTEXT("AttributeFeedback", "CoinSuffix", ""), Style.ZOffset, Style.Color, Style.OutlineColor);
}

void UMAAttributeFeedbackComponent::ShowPositiveAttributeDelta(float Delta, const FText& Suffix, float ZOffset, const FLinearColor& Color, const FLinearColor& OutlineColor) const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;

	AMAPlayerController* PlayerController = Cast<AMAPlayerController>(OwnerPawn->GetController());
	if (!PlayerController) return;

	const FText Text = FText::Format(NSLOCTEXT("AttributeFeedback", "PositiveDelta", "+{0}{1}"), FText::AsNumber(FMath::RoundToInt(Delta)), Suffix);
	PlayerController->ShowFloatingText(Text, OwnerPawn->GetActorLocation() + FVector(0.f, 0.f, ZOffset), Color, OutlineColor);
}
