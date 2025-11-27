// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/MovableWindowWidget.h"
#include "Abilities/GameplayAbility.h"
#include "SkillBookWidget.generated.h"

class UWrapBox;
class USkillSlotWidget;
class USkillBookComponent;

UCLASS()
class USkillBookWidget : public UMovableWindowWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnSkillLearned(TSubclassOf<UGameplayAbility> NewSkillClass);

	void AddSkillSlot(TSubclassOf<UGameplayAbility> SkillClass);
	
	UPROPERTY(meta = (BindWidget))
	UWrapBox* SkillList;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USkillSlotWidget> SlotWidgetClass;
	
	UPROPERTY()
	USkillBookComponent* SkillBookComponent;
};