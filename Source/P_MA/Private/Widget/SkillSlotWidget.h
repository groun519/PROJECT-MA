// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Abilities/GameplayAbility.h"
#include "Inventory/MAItemTypes.h" 
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
	
	const struct FSkillItemData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

private:
	UPROPERTY()
	TSubclassOf<UGameplayAbility> SkillClass;

	UPROPERTY(meta = (BindWidget))
	class UImage* SkillIcon;
	
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* AbilityDataTable;
    
	virtual void NativeOnDragDetected( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation ) override;
	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	TSubclassOf<class UUserWidget> DragVisualClass;
};