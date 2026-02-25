// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilityListView.h"
#include "Abilities/GameplayAbility.h"
#include "Widget/MAAbilityGauge.h"
#include "Widget/SkillSlotWidget.h"


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
		DataItem->AbilityClass = Abilities.Contains(TargetInputID) ? Abilities[TargetInputID] : nullptr;

		// 데이터만 추가하면, SkillSlotWidget 내부의 NativeOnListItemObjectSet이 자동 실행
		AddItem(DataItem);
	}
}
