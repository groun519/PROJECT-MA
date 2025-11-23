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

	// 1. 아이콘 로드 (비동기 로드 권장되지만 일단 동기 로드)
	if (UTexture2D* IconTexture = ItemDataObject->CachedItemData->Icon.LoadSynchronous())
	{
		SetIcon(IconTexture);
	}
	
	// 2. (선택사항) 툴팁이나 가격 표시 로직 추가 가능
	// SetPrice(ItemDataObject->CachedItemData->Price);
}

void UShopItemWidget::RightButtonClicked()
{
	if (ItemDataObject)
	{
		// 구매 요청 발생 -> ShopWidget이 받음
		OnItemPurchaseIssued.Broadcast(ItemDataObject);
	}
}

void UShopItemWidget::LeftButtonClicked()
{
	// 클릭 효과 등
}