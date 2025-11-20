// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SkillBookWidget.h"
#include "Widget/SkillSlotWidget.h"
#include "Inventory/SkillBookComponent.h"
#include "Components/WrapBox.h"
#include "Player/MAPlayerCharacter.h" // 캐릭터 헤더 필요

void USkillBookWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!SkillList) return;
	SkillList->ClearChildren(); // 초기화

	// 플레이어 캐릭터에서 SkillBookComponent 찾기
	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		if (AMAPlayerCharacter* MAChar = Cast<AMAPlayerCharacter>(OwnerPawn))
		{
			SkillBookComponent = MAChar->GetSkillBookComponent();
		}
	}

	if (SkillBookComponent)
	{
		// 1. 이미 배운 스킬들을 UI에 표시 (로드 시점 등)
		for (const auto& SkillClass : SkillBookComponent->GetLearnedSkills())
		{
			AddSkillSlot(SkillClass);
		}

		// 2. 앞으로 배울 스킬들에 대해 이벤트 구독
		SkillBookComponent->OnSkillLearned.AddDynamic(this, &USkillBookWidget::OnSkillLearned);
	}
}

void USkillBookWidget::OnSkillLearned(TSubclassOf<UGameplayAbility> NewSkillClass)
{
	AddSkillSlot(NewSkillClass);
}

void USkillBookWidget::AddSkillSlot(TSubclassOf<UGameplayAbility> SkillClass)
{
	if (!SlotWidgetClass || !SkillList) return;

	// 슬롯 위젯 생성
	USkillSlotWidget* NewSlot = CreateWidget<USkillSlotWidget>(this, SlotWidgetClass);
	if (NewSlot)
	{
		NewSlot->Init(SkillClass);
		SkillList->AddChildToWrapBox(NewSlot);
	}
}

