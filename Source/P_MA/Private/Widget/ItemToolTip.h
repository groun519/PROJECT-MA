// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemToolTip.generated.h"

class UPA_ShopItem;
/**
 * 
 */
UCLASS()
class UItemToolTip : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetItem(const UPA_ShopItem* Item);
	void SetPrice(float newPrice);
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* IconImage;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ItemTitleText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ItemDescriptionText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ItemPriceText;
};