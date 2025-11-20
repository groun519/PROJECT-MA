// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MAAbilitySlotWidget.generated.h"

class UImage;
class UGameplayAbility;

UCLASS()
class UMAAbilitySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass);

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	// 이 슬롯이 담당하는 키 (에디터에서 Q, E, R 등으로 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	EMAAbilityInputID AssignedInputID;

private:
	UPROPERTY(meta = (BindWidget))
	UImage* SkillIcon;
};