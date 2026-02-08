// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadoutColorButtonWidget.generated.h"

class UButton;
class UImage;

UCLASS()
class P_MA_API ULoadoutColorButtonWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorSelected, FMaterialParamData, SelectedData);

	/** Input Color **/
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ColorButton;

	/** Selected Frame **/
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> EquippedBorder;

	void SetSelected(bool bInSelected);

	/** Data **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loadout|Color")
	FMaterialParamData ColorData;

	/** Delegate **/
	UPROPERTY(BlueprintAssignable, Category="Loadout|Color")
	FOnColorSelected OnColorSelected; // 색 클릭 이벤트 전송

private:
	UFUNCTION()
	void HandleColorClicked();

	bool bSelected = false;
};
