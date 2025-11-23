// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Abilities/GameplayAbility.h"
#include "Inventory/MAItemTypes.h" // [필수] 이제 이 헤더를 사용합니다!
#include "SkillSlotWidget.generated.h"

// FAbilityWidgetData 전방 선언 제거
// class UDataTable; // 아래에 포함되어 있으므로 생략 가능

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

	// [변경] 반환 타입 수정: FAbilityWidgetData -> FSkillItemData
	const struct FSkillItemData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

private:
	UPROPERTY()
	TSubclassOf<UGameplayAbility> SkillClass;

	UPROPERTY(meta = (BindWidget))
	class UImage* SkillIcon;

	// 에디터에서 DT_Skills를 넣어줄 변수
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* AbilityDataTable;
    
	virtual void NativeOnDragDetected( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation ) override;
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	TSubclassOf<class UUserWidget> DragVisualClass;
};