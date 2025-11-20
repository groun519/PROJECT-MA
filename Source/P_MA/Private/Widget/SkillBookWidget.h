// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Abilities/GameplayAbility.h"
#include "SkillBookWidget.generated.h"

class UWrapBox;
class USkillSlotWidget;
class USkillBookComponent;

UCLASS()
class USkillBookWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

private:
	// 스킬이 추가되었을 때 호출될 함수 (델리게이트 바인딩용)
	UFUNCTION()
	void OnSkillLearned(TSubclassOf<UGameplayAbility> NewSkillClass);

	// 슬롯을 생성해서 화면에 붙이는 헬퍼 함수
	void AddSkillSlot(TSubclassOf<UGameplayAbility> SkillClass);

	// UI 바인딩
	UPROPERTY(meta = (BindWidget))
	UWrapBox* SkillList;

	// 생성할 슬롯 위젯 클래스 (BP에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USkillSlotWidget> SlotWidgetClass;

	// 데이터 컴포넌트 참조
	UPROPERTY()
	USkillBookComponent* SkillBookComponent;
};