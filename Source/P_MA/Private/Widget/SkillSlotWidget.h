// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAGameplayAbilityTypes.h" // 💡 EMAAbilityInputID 사용을 위해 추가
#include "Inventory/MAItemTypes.h" 
#include "SkillSlotWidget.generated.h"

UCLASS()
class USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(TSubclassOf<UGameplayAbility> NewSkillClass, EMAAbilityInputID NewInputID);
	
	TSubclassOf<UGameplayAbility> GetSkillClass() const { return SkillClass; }
	
	UFUNCTION(BlueprintPure, Category = "UI")
	FString GetShortKeyName(FKey Key) const;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnSkillSet(TSubclassOf<UGameplayAbility> NewSkillClass);
	
	const struct FSkillData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

private:
	UPROPERTY()
	TSubclassOf<UGameplayAbility> SkillClass;

	UPROPERTY(meta = (BindWidget))
	class UImage* SkillIcon;
	
	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	class UTextBlock* HotkeyText;
	
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* AbilityDataTable;
    
	virtual void NativeOnDragDetected( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation ) override;
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
};