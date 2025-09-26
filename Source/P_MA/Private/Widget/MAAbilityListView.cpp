// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilityListView.h"
#include "Abilities/GameplayAbility.h"
#include "Widget/MAAbilityGauge.h"

void UMAAbilityListView::ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities)
{
	OnEntryWidgetGenerated().AddUObject(this, &UMAAbilityListView::AbilityGaugeGenerated);
	for (const TPair<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		AddItem(AbilityPair.Value.GetDefaultObject());
	}
}

void UMAAbilityListView::AbilityGaugeGenerated(UUserWidget& Widget)
{
	UMAAbilityGauge* AbilityGauge = Cast<UMAAbilityGauge>(&Widget);

	if (AbilityGauge)
	{
		AbilityGauge->ConfigureWithWidgetData(FindWidgetDataForAbility(AbilityGauge->GetListItem<UGameplayAbility>()->GetClass()));
	}
}

const FAbilityWidgetData* UMAAbilityListView::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable)
		return nullptr;

	for (auto& AbilityWidgetDataPair : AbilityDataTable->GetRowMap())
	{
		const FAbilityWidgetData* WidgetData = AbilityDataTable->FindRow<FAbilityWidgetData>(AbilityWidgetDataPair.Key, "");
		if (WidgetData->AbilityClass == AbilityClass)
		{
			return WidgetData;
		}
	}

	return nullptr;
}