// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"

class UItemToolTip;
class UPA_ShopItem;

/**
 * 
 */
UCLASS()
class UItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void SetIcon(UTexture2D* IconTexture);

protected:
	UItemToolTip* SetToolTipWidget(const UPA_ShopItem* Item);
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* ItemIcon;

	UPROPERTY(EditDefaultsOnly, Category = "ToolTip")
	TSubclassOf<UItemToolTip> ItemToolTipClass;

	virtual FReply NativeOnMouseButtonDown( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;
	virtual FReply NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;

	virtual void RightButtonClicked();
	virtual void LeftButtonClicked();
};