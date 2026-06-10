#include "Widget/MAValueGauge.h"

#include "AbilitySystemComponent.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UMAValueGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	HealthFillImage->SetColorAndOpacity(HealthColor);
	ShieldFillImage->SetColorAndOpacity(ShieldColor);
	
	FSlateFontInfo HealthFont = HealthText->GetFont();
	HealthFont.Size = ValueTextSize;
	HealthText->SetFont(HealthFont);

	FSlateFontInfo ShieldFont = ShieldText->GetFont();
	ShieldFont.Size = ValueTextSize;
	ShieldText->SetFont(ShieldFont);
}

void UMAValueGauge::Bind3Attributes(
	UAbilitySystemComponent* ASC,
	const FGameplayAttribute& HealthAttribute,
	const FGameplayAttribute& MaxHealthAttribute,
	const FGameplayAttribute& ShieldAttribute)
{
	if (BoundASC.Get() == ASC) return;
	BoundASC = ASC;

	if (ASC)
	{
		bool bFound;
		float Health	= ASC->GetGameplayAttributeValue(HealthAttribute, bFound);
		float MaxHealth = ASC->GetGameplayAttributeValue(MaxHealthAttribute, bFound);
		float Shield	= ASC->GetGameplayAttributeValue(ShieldAttribute, bFound);
		if (bFound) Set3Values(Health, MaxHealth, Shield);

		ASC->GetGameplayAttributeValueChangeDelegate(HealthAttribute).AddUObject(this, &UMAValueGauge::HealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(MaxHealthAttribute).AddUObject(this, &UMAValueGauge::MaxHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(ShieldAttribute).AddUObject(this, &UMAValueGauge::ShieldChanged);
	}
}

void UMAValueGauge::SetFillRatio(UWidget* FillRoot, float FillRatio)
{
	UHorizontalBoxSlot* HorizontalBoxSlot = CastChecked<UHorizontalBoxSlot>(FillRoot->Slot);

	FSlateChildSize Size(ESlateSizeRule::Fill);
	Size.Value = FMath::Max(FillRatio, 0.f);

	HorizontalBoxSlot->SetSize(Size);
}

void UMAValueGauge::Set3Values(float NewHealth, float NewMaxHealth, float NewShield)
{
	CachedHealth = NewHealth;
	CachedMaxHealth = NewMaxHealth;
	CachedShield = NewShield;

	const float EmptyValue = FMath::Max(NewMaxHealth - NewHealth, 0.f);
	const float TotalValue = FMath::Max(NewMaxHealth + NewShield, KINDA_SMALL_NUMBER);

	SetFillRatio(HealthFillRoot, NewHealth / TotalValue);
	SetFillRatio(EmptyFillSpacer, EmptyValue / TotalValue);
	SetFillRatio(ShieldFillRoot, NewShield / TotalValue);

	// 소수점x
	FNumberFormattingOptions NoDecimalFormat =
		FNumberFormattingOptions().SetMaximumFractionalDigits(0);
	
	HealthText->SetVisibility(bShowValueText ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	HealthText->SetText(FText::AsNumber(NewHealth, &NoDecimalFormat));

	ShieldText->SetVisibility(bShowValueText && NewShield > 0.f ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ShieldText->SetText(FText::AsNumber(NewShield, &NoDecimalFormat));
}

void UMAValueGauge::HealthChanged(const FOnAttributeChangeData& ChangedData)
{
	Set3Values(ChangedData.NewValue, CachedMaxHealth, CachedShield);
}

void UMAValueGauge::MaxHealthChanged(const FOnAttributeChangeData& ChangedData)
{
	Set3Values(CachedHealth, ChangedData.NewValue, CachedShield);
}

void UMAValueGauge::ShieldChanged(const FOnAttributeChangeData& ChangedData)
{
	Set3Values(CachedHealth, CachedMaxHealth, ChangedData.NewValue);
}
