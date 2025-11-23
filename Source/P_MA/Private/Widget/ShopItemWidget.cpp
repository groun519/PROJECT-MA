// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/ShopItemWidget.h"
#include "Components/Image.h" 

void UShopItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	ItemDataObject = Cast<UShopItemDataObject>(ListItemObject);
    
	if (!ItemDataObject || !ItemDataObject->CachedItemData)
	{
		return;
	}
	
	if (UTexture2D* IconTexture = ItemDataObject->CachedItemData->Icon.LoadSynchronous())
	{
		SetIcon(IconTexture);
	}
	
}

void UShopItemWidget::RightButtonClicked()
{
	if (ItemDataObject)
	{
		OnItemPurchaseIssued.Broadcast(ItemDataObject);
	}
}

void UShopItemWidget::LeftButtonClicked()
{
	
}