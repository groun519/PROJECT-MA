// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilityListView.h"
#include "Abilities/GameplayAbility.h"
#include "Widget/MAAbilityGauge.h"

void UMAAbilityListView::ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities)
{
	ClearListItems();

	// [변경] 등록된 스킬만 추가하는 것이 아니라, 'Skill1' ~ 'Skill3'까지 3개의 고정 슬롯을 생성합니다.
	// (프로젝트의 InputID 정의에 따라 Skill1, Skill2, Skill3 순서대로 루프를 돌거나 배열을 만드세요)
	TArray<EMAAbilityInputID> TargetSlots = { 
		EMAAbilityInputID::Skill1, 
		EMAAbilityInputID::Skill2, 
		EMAAbilityInputID::Skill3 
	};

	for (EMAAbilityInputID TargetInputID : TargetSlots)
	{
		// 1. 데이터 객체 생성
		UMAAbilitySlotDataObject* DataItem = NewObject<UMAAbilitySlotDataObject>(this);
		DataItem->InputID = TargetInputID;

		// 2. 해당 키에 등록된 스킬이 있는지 확인
		if (Abilities.Contains(TargetInputID))
		{
			DataItem->AbilityClass = Abilities[TargetInputID];
		}
		else
		{
			DataItem->AbilityClass = nullptr; // 스킬이 없어도 데이터 객체는 생성 (빈 슬롯)
		}

		// 3. 리스트에 추가 (이 시점에 위젯이 생성되거나 갱신됨)
		AddItem(DataItem);
	}
}