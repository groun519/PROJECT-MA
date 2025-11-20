// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Abilities/GameplayAbility.h"
#include "SkillSlotWidget.generated.h"

UCLASS()
class USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(TSubclassOf<UGameplayAbility> NewSkillClass);
	TSubclassOf<UGameplayAbility> GetSkillClass() const { return SkillClass; }

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnSkillSet(TSubclassOf<UGameplayAbility> NewSkillClass);

	// [+++ 추가 +++] 드래그 시작 감지
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	// [+++ 추가 +++] 마우스 클릭 감지
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UPROPERTY()
	TSubclassOf<UGameplayAbility> SkillClass;

	UPROPERTY(meta = (BindWidget))
	class UImage* SkillIcon;
    
	// [+++ 추가 +++] 드래그 시각 효과 위젯 클래스 (블루프린트에서 설정)
	UPROPERTY(EditDefaultsOnly, Category = "DragDrop")
	TSubclassOf<UUserWidget> DragVisualClass;
};