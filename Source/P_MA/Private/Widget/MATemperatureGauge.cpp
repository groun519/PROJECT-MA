#include "Widget/MATemperatureGauge.h"

#include "AbilitySystemComponent.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Spacer.h"
#include "Components/Widget.h"
#include "GAS/MAAttributeSet.h"

void UMATemperatureGauge::NativePreConstruct()
{
	Super::NativePreConstruct();

	ApplySectionRatio();
	ApplyTemperatureVisuals();
}

void UMATemperatureGauge::NativeDestruct()
{
	UnbindTemperatureAttribute();
	Super::NativeDestruct();
}

void UMATemperatureGauge::BindTemperatureAttribute(UAbilitySystemComponent* ASC)
{
	if (BoundASC.Get() == ASC) return;

	UnbindTemperatureAttribute();
	BoundASC = ASC;
	if (!ASC) return;

	SetTemperature(ASC->GetNumericAttribute(UMAAttributeSet::GetTemperatureAttribute()));
	TemperatureChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetTemperatureAttribute())
		.AddUObject(this, &UMATemperatureGauge::HandleTemperatureChanged);
}

void UMATemperatureGauge::SetCriticalRatio(float NewCriticalRatio)
{
	CriticalRatio = FMath::Clamp(NewCriticalRatio, 0.f, 1.f);
	ApplySectionRatio();
}

void UMATemperatureGauge::SetTemperature(float NewTemperature)
{
	Temperature = FMath::Clamp(NewTemperature, -MaxAbsTemperature, MaxAbsTemperature);
	ApplyTemperatureVisuals();
}

void UMATemperatureGauge::ApplySectionRatio() const
{
	const float ClampedCriticalRatio = FMath::Clamp(CriticalRatio, 0.f, 1.f);
	SetFillRatio(NormalImage, 1.f - ClampedCriticalRatio);
	SetFillRatio(CriticalImage, ClampedCriticalRatio);
}

void UMATemperatureGauge::ApplyTemperatureVisuals() const
{
	const float AbsAlpha = FMath::Clamp(FMath::Abs(Temperature) / FMath::Max(MaxAbsTemperature, KINDA_SMALL_NUMBER), 0.f, 1.f);

	if (NormalImage) NormalImage->SetColorAndOpacity(ResolveNormalTemperatureColor());
	if (CriticalImage) CriticalImage->SetColorAndOpacity(ResolveCriticalTemperatureColor());
	if (TemperatureFillImage) TemperatureFillImage->SetColorAndOpacity(ResolveProgressTemperatureColor());
	SetFillRatio(TemperatureFillImage, AbsAlpha);
	SetFillRatio(EmptyFillSpacer, 1.f - AbsAlpha);
}

void UMATemperatureGauge::HandleTemperatureChanged(const FOnAttributeChangeData& Data)
{
	SetTemperature(Data.NewValue);
}

void UMATemperatureGauge::UnbindTemperatureAttribute()
{
	if (UAbilitySystemComponent* ASC = BoundASC.Get())
	{
		if (TemperatureChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetTemperatureAttribute()).Remove(TemperatureChangedHandle);
		}
	}

	TemperatureChangedHandle.Reset();
	BoundASC.Reset();
}

FLinearColor UMATemperatureGauge::ResolveNormalTemperatureColor() const
{
	return ResolveTemperatureColorBySet(
		NormalNegativeTemperatureColor,
		NormalZeroTemperatureColor,
		NormalPositiveTemperatureColor);
}

FLinearColor UMATemperatureGauge::ResolveCriticalTemperatureColor() const
{
	return ResolveTemperatureColorBySet(
		CriticalNegativeTemperatureColor,
		CriticalZeroTemperatureColor,
		CriticalPositiveTemperatureColor);
}

FLinearColor UMATemperatureGauge::ResolveProgressTemperatureColor() const
{
	return ResolveTemperatureColorBySet(
		ProgressNegativeTemperatureColor,
		ProgressZeroTemperatureColor,
		ProgressPositiveTemperatureColor);
}

FLinearColor UMATemperatureGauge::ResolveTemperatureColorBySet(
	const FLinearColor& NegativeColor,
	const FLinearColor& ZeroColor,
	const FLinearColor& PositiveColor) const
{
	if (FMath::IsNearlyZero(Temperature)) return ZeroColor;
	return Temperature > 0.f ? PositiveColor : NegativeColor;
}

void UMATemperatureGauge::SetFillRatio(UWidget* Widget, float FillRatio) const
{
	if (!Widget) return;

	if (UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(Widget->Slot))
	{
		FSlateChildSize Size(ESlateSizeRule::Fill);
		Size.Value = FMath::Max(FillRatio, 0.f);
		HorizontalBoxSlot->SetSize(Size);
	}
}
