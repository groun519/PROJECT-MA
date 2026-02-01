// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutColorButtonWidget.generated.h"

class UButton;

UCLASS()
class P_MA_API ULoadoutColorButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorSelected, FLinearColor, SelectedColor);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ColorButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loadout|Color")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(BlueprintAssignable, Category="Loadout|Color")
	FOnColorSelected OnColorSelected;

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleColorClicked();
};
