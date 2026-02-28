// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FieldItemInteractWidget.generated.h"

class AMAFieldItem;

UCLASS()
class P_MA_API UFieldItemInteractWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 자신이 속한 3D 액터의 정보를 담을 포인터
	UPROPERTY()
	AMAFieldItem* OwnerFieldItem;

	virtual void NativeConstruct() override;

protected:
	// 1. 마우스 왼쪽 버튼을 누르면 "드래그 감지 모드"로 진입
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	// 2. 실제로 마우스를 끌면 드래그 오퍼레이션 생성
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	UPROPERTY()
	class UUserWidget* CachedToolTipWidget;
};
