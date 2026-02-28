// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/SkillSlotWidget.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"
#include "Widget/SkillDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"
#include "Inventory/MAItemTypes.h" 

void USkillSlotWidget::Init(TSubclassOf<UGameplayAbility> NewSkillClass, EMAAbilityInputID NewInputID)
{
	this->SkillClass = NewSkillClass;
	const FSkillItemData* WidgetData = FindWidgetDataForAbility(NewSkillClass);

	if (WidgetData && SkillIcon)
	{
		UTexture2D* Texture = WidgetData->Icon.LoadSynchronous();
		if (Texture)
		{
			SkillIcon->SetBrushFromTexture(Texture);
			SkillIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}
	
	if (HotkeyText)
	{
		HotkeyText->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnSkillSet(NewSkillClass);
}

const FSkillItemData* USkillSlotWidget::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable) return nullptr;
	
	for (auto& RowPair : AbilityDataTable->GetRowMap())
	{
		const FSkillItemData* Data = reinterpret_cast<const FSkillItemData*>(RowPair.Value);
		
		if (Data && Data->GrantedAbility == AbilityClass)
		{
			return Data;
		}
	}
	return nullptr;
}


FReply USkillSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USkillSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (SkillClass)
	{
		USkillDragDropOperation* DragOp = NewObject<USkillDragDropOperation>();
		DragOp->SkillClass = SkillClass;

		if (SkillIcon) 
		{
			DragOp->DefaultDragVisual = this;
			DragOp->Pivot = EDragPivot::CenterCenter;
		}

		OutOperation = DragOp;
	}
}
FString USkillSlotWidget::GetShortKeyName(FKey Key) const
{
	// 스킬북 목록 위젯에서는 사실 이 함수를 쓸 일이 없지만 링킹 에러 방지를 위해
	return Key.GetDisplayName().ToString();
}