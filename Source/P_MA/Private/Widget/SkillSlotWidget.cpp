// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/SkillSlotWidget.h"
#include "Components/Image.h"
#include "Widget/SkillDragDropOperation.h" // 헤더 추가
#include "Blueprint/WidgetBlueprintLibrary.h" // 헤더 추가

void USkillSlotWidget::Init(TSubclassOf<UGameplayAbility> NewSkillClass)
{
	SkillClass = NewSkillClass;
	OnSkillSet(NewSkillClass);
}

// [+++ 추가 +++] 좌클릭 시 드래그 감지 요청
FReply USkillSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

// [+++ 추가 +++] 드래그 시작 시 오퍼레이션 생성
void USkillSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (SkillClass)
	{
		USkillDragDropOperation* DragOp = NewObject<USkillDragDropOperation>();
		DragOp->SkillClass = SkillClass;

		if (DragVisualClass)
		{
			UUserWidget* DragVisual = CreateWidget<UUserWidget>(GetWorld(), DragVisualClass);
			// 필요하다면 여기서 DragVisual에 아이콘 등을 세팅 (ex: DragVisual->SetIcon(...))
			DragOp->DefaultDragVisual = DragVisual;
			DragOp->Pivot = EDragPivot::CenterCenter;
		}

		OutOperation = DragOp;
	}
}