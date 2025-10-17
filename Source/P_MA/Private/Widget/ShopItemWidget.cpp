// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/ShopItemWidget.h"
#include "Inventory/PA_ShopItem.h"

void UShopItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	ShopItem = Cast<UPA_ShopItem>(ListItemObject);
	if (!ShopItem)
	{
		return;
	}

	SetIcon(ShopItem->GetIcon());
	SetToolTipWidget(ShopItem);
}

void UShopItemWidget::RightButtonClicked()
{
	OnItemPurchaseIssued.Broadcast(GetShopItem());
}

void UShopItemWidget::LeftButtonClicked()
{
	OnShopItemClicked.Broadcast(this);
}
