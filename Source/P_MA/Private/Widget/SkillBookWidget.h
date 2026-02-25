// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/MovableWindowWidget.h"
#include "Abilities/GameplayAbility.h"
#include "SkillBookWidget.generated.h"

class UWrapBox;
class USkillSlotWidget;
class USkillBookComponent;
class UButton; 

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillBookCloseRequested);

UCLASS()
class USkillBookWidget : public UMovableWindowWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillBookCloseRequested OnCloseRequested;

private:
	UFUNCTION()
	void OnSkillLearned(TSubclassOf<UGameplayAbility> NewSkillClass);
    
	UFUNCTION()
	void OnCloseClicked();

	void AddSkillSlot(TSubclassOf<UGameplayAbility> SkillClass);
    
	UPROPERTY(meta = (BindWidget))
	UWrapBox* SkillList;
    
	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USkillSlotWidget> SlotWidgetClass;
    
	UPROPERTY()
	USkillBookComponent* SkillBookComponent;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* SkillBookAnim;
};