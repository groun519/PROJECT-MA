// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilityListView.h"
#include "Abilities/GameplayAbility.h"
#include "Widget/MAAbilityGauge.h"

void UMAAbilityListView::ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities)
{
	ClearListItems();
	
	TArray<EMAAbilityInputID> TargetSlots = { 
		EMAAbilityInputID::Skill1, 
		EMAAbilityInputID::Skill2, 
		EMAAbilityInputID::Skill3 
	};

	for (EMAAbilityInputID TargetInputID : TargetSlots)
	{
		UMAAbilitySlotDataObject* DataItem = NewObject<UMAAbilitySlotDataObject>(this);
		DataItem->InputID = TargetInputID;

		if (Abilities.Contains(TargetInputID))
		{
			DataItem->AbilityClass = Abilities[TargetInputID];
		}
		else
		{
			DataItem->AbilityClass = nullptr;
		}
		AddItem(DataItem);
	}
}