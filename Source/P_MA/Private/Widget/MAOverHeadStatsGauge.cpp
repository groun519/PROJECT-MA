#include "Widget/MAOverHeadStatsGauge.h"

#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "Character/MAStatusEffectComponent.h"
#include "GAS/MAAttributeSet.h"
#include "Widget/MAStatusEffectDurationWidget.h"
#include "Widget/MATemperatureGauge.h"
#include "Widget/MAValueGauge.h"

void UMAOverHeadStatsGauge::InitializeFromCharacter(AMACharacter* OwnerCharacter)
{
	ConfigureWithASC(OwnerCharacter ? OwnerCharacter->GetAbilitySystemComponent() : nullptr);
	ConfigureWithStatusEffectComponent(OwnerCharacter ? OwnerCharacter->GetStatusEffectComponent() : nullptr);
}

void UMAOverHeadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent) return;

	HealthBar->Bind3Attributes(
		AbilitySystemComponent,
		UMAAttributeSet::GetHealthAttribute(),
		UMAAttributeSet::GetMaxHealthAttribute(),
		UMAAttributeSet::GetShieldAttribute());

	if (TemperatureBar)
	{
		TemperatureBar->BindTemperatureAttribute(AbilitySystemComponent);
	}
}

void UMAOverHeadStatsGauge::ConfigureWithStatusEffectComponent(UMAStatusEffectComponent* StatusEffectComponent)
{
	if (BoundStatusEffectComponent.Get() == StatusEffectComponent) return;

	UnbindFromStatusEffectComponent();
	BoundStatusEffectComponent = StatusEffectComponent;
	if (!StatusEffectComponent)
	{
		ClearStatusEffectSlots();
		return;
	}

	StatusEffectComponent->OnStatusEffectDisplayChanged.AddUObject(this, &UMAOverHeadStatsGauge::RefreshStatusEffectDisplay);
	RefreshStatusEffectDisplay();
}

void UMAOverHeadStatsGauge::UnbindFromStatusEffectComponent()
{
	if (BoundStatusEffectComponent.IsValid())
	{
		BoundStatusEffectComponent->OnStatusEffectDisplayChanged.RemoveAll(this);
	}
}

TArray<UMAStatusEffectDurationWidget*> UMAOverHeadStatsGauge::GetStatusEffectSlots() const
{
	TArray<UMAStatusEffectDurationWidget*> Slots;
	Slots.Reserve(5);

	Slots.Add(StatusEffectSlot0);
	Slots.Add(StatusEffectSlot1);
	Slots.Add(StatusEffectSlot2);
	Slots.Add(StatusEffectSlot3);
	Slots.Add(StatusEffectSlot4);

	return Slots;
}

void UMAOverHeadStatsGauge::ClearStatusEffectSlots() const
{
	for (UMAStatusEffectDurationWidget* SlotWidget : GetStatusEffectSlots())
	{
		SlotWidget->ClearStatusEffectDuration();
	}
}

void UMAOverHeadStatsGauge::RefreshStatusEffectDisplay()
{
	ClearStatusEffectSlots();

	if (!BoundStatusEffectComponent.IsValid()) return;

	TArray<FStatusEffectDisplayEvent> ActiveStatusEffectEvents;
	BoundStatusEffectComponent->GetActiveStatusEffectDisplayEvents(ActiveStatusEffectEvents);
	const TArray<UMAStatusEffectDurationWidget*> SlotWidgets = GetStatusEffectSlots();
	const int32 DisplayCount = FMath::Min(ActiveStatusEffectEvents.Num(), SlotWidgets.Num());

	for (int32 Index = 0; Index < DisplayCount; ++Index)
	{
		SlotWidgets[Index]->SetStatusEffectDuration(
			ActiveStatusEffectEvents[Index].Label,
			ActiveStatusEffectEvents[Index].Duration,
			ActiveStatusEffectEvents[Index].RemainingDuration);
	}
}
