// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/SkillSlotWidget.h"
#include "Components/Image.h"
#include "Engine/DataTable.h"
#include "Widget/SkillDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Inventory/MAItemTypes.h" // [필수]

void USkillSlotWidget::Init(TSubclassOf<UGameplayAbility> NewSkillClass)
{
	SkillClass = NewSkillClass;
    
	// [변경] 새로운 구조체로 데이터 검색
	const FSkillItemData* WidgetData = FindWidgetDataForAbility(NewSkillClass);

	if (WidgetData && SkillIcon)
	{
		// [변경] 아이콘 로드 (FBaseItemData에 Icon이 있음)
		UTexture2D* Texture = WidgetData->Icon.LoadSynchronous();
		if (Texture)
		{
			SkillIcon->SetBrushFromTexture(Texture);
			SkillIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if(SkillIcon) 
		{
			// 아이콘이 없으면 숨김 처리하거나 기본 이미지
			// SkillIcon->SetVisibility(ESlateVisibility::Hidden); 
		}
	}

	OnSkillSet(NewSkillClass);
}

// [핵심 변경] 검색 로직 수정
const FSkillItemData* USkillSlotWidget::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable) return nullptr;

	// 테이블의 모든 행을 순회
	for (auto& RowPair : AbilityDataTable->GetRowMap())
	{
		// [중요] FSkillItemData로 캐스팅해야 합니다! (옛날 FAbilityWidgetData 아님)
		const FSkillItemData* Data = reinterpret_cast<const FSkillItemData*>(RowPair.Value);
		
		// 데이터가 유효하고, GrantedAbility(배우는 스킬)가 내가 찾는 스킬 클래스와 같다면?
		if (Data && Data->GrantedAbility == AbilityClass)
		{
			return Data;
		}
	}
	return nullptr;
}

// ... (아래 드래그 앤 드롭 관련 함수들은 기존과 동일하게 유지) ...

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