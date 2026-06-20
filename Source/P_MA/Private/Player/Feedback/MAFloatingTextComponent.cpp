#include "Player/Feedback/MAFloatingTextComponent.h"

#include "AbilitySystemInterface.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Passive/MAFloatingTextActor.h"
#include "GameFramework/Pawn.h"
#include "Player/MAPlayerController.h"
#include "Setting/MAGameSettings.h"

UMAFloatingTextComponent::UMAFloatingTextComponent()
{
	SetIsReplicatedByDefault(true);
}

void UMAFloatingTextComponent::BindToPawn(APawn* Pawn)
{
	UnbindASC();

	const AMAPlayerController* PlayerController = Cast<AMAPlayerController>(GetOwner());
	if (!PlayerController || !PlayerController->IsLocalController() || !Pawn) return;

	const IAbilitySystemInterface* AbilityOwner = Cast<IAbilitySystemInterface>(Pawn);
	BoundASC = AbilityOwner ? Cast<UMAAbilitySystemComponent>(AbilityOwner->GetAbilitySystemComponent()) : nullptr;
	if (UMAAbilitySystemComponent* ASC = BoundASC.Get())
	{
		ShieldChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetShieldAttribute())
			.AddUObject(this, &UMAFloatingTextComponent::HandleShieldChanged);
		CoinChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetCoinAttribute())
			.AddUObject(this, &UMAFloatingTextComponent::HandleCoinChanged);
	}
}

void UMAFloatingTextComponent::ClientShowDamage_Implementation(
	float Amount,
	AActor* TargetActor,
	EMADamageCriticalResult CriticalResult,
	bool bIsIncoming,
	FGameplayTag DamageTypeTag)
{
	if (!TargetActor) return;

	const FMADamageTextStyle Style = UMAGameSettings::Get()->GetDamageTextStyle(
		DamageTypeTag,
		CriticalResult,
		bIsIncoming);
	FVector TextLocation = TargetActor->GetActorLocation() + FVector(0.f, 0.f, Style.ZOffset);
	TextLocation.X += FMath::RandRange(-40.f, 40.f);
	TextLocation.Y += FMath::RandRange(-40.f, 40.f);

	ShowLocal(
		Amount > 0.f
			? FText::AsNumber(FMath::RoundToInt(Amount))
			: NSLOCTEXT("DamageText", "Miss", "MISS"),
		TextLocation,
		Style.Color,
		Style.OutlineColor,
		FMath::GetMappedRangeValueClamped(FVector2D(10.f, 1000.f), FVector2D(0.75f, 2.f), Amount));
}

void UMAFloatingTextComponent::ShowLocal(
	const FText& Text,
	const FVector& WorldLocation,
	const FLinearColor& Color,
	const FLinearColor& OutlineColor,
	float Scale) const
{
	if (!FloatingTextActorClass || !GetWorld()) return;

	if (AMAFloatingTextActor* FloatingTextActor = GetWorld()->SpawnActor<AMAFloatingTextActor>(
		FloatingTextActorClass,
		WorldLocation,
		FRotator::ZeroRotator))
	{
		FloatingTextActor->PlayText(Text, Color, OutlineColor, Scale);
	}
}

void UMAFloatingTextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindASC();
	Super::EndPlay(EndPlayReason);
}

void UMAFloatingTextComponent::UnbindASC()
{
	if (UMAAbilitySystemComponent* ASC = BoundASC.Get())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetShieldAttribute()).Remove(ShieldChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetCoinAttribute()).Remove(CoinChangedHandle);
	}

	BoundASC.Reset();
	ShieldChangedHandle.Reset();
	CoinChangedHandle.Reset();
}

void UMAFloatingTextComponent::HandleShieldChanged(const FOnAttributeChangeData& Data)
{
	const float Delta = Data.NewValue - Data.OldValue;
	if (Delta <= 0.f) return;

	const FMADamageTextStyle& Style = UMAGameSettings::Get()->ShieldAttributeFeedbackTextStyle;
	ShowPositiveAttributeDelta(Delta, Style.Color, Style.OutlineColor, Style.ZOffset);
}

void UMAFloatingTextComponent::HandleCoinChanged(const FOnAttributeChangeData& Data)
{
	const float Delta = Data.NewValue - Data.OldValue;
	if (Delta <= 0.f) return;

	const FMADamageTextStyle& Style = UMAGameSettings::Get()->CoinAttributeFeedbackTextStyle;
	ShowPositiveAttributeDelta(Delta, Style.Color, Style.OutlineColor, Style.ZOffset);
}

void UMAFloatingTextComponent::ShowPositiveAttributeDelta(
	float Delta,
	const FLinearColor& Color,
	const FLinearColor& OutlineColor,
	float ZOffset) const
{
	const AMAPlayerController* PlayerController = Cast<AMAPlayerController>(GetOwner());
	const APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!Pawn) return;

	ShowLocal(
		FText::Format(NSLOCTEXT("AttributeFeedback", "PositiveDelta", "+{0}"), FText::AsNumber(FMath::RoundToInt(Delta))),
		Pawn->GetActorLocation() + FVector(0.f, 0.f, ZOffset),
		Color,
		OutlineColor);
}
