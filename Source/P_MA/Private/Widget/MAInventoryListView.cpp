// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAInventoryListView.h"

void UMAInventoryListView::UpdateInventoryList(const TArray<UInventoryItem*>& InventoryItems, int32 Capacity)
{
	ClearListItems();
	
	for (UInventoryItem* Item : InventoryItems)
	{
		if (Item && Item->IsValid())
		{
			UMAInventorySlotDataObject* DataObj = NewObject<UMAInventorySlotDataObject>(this);
			DataObj->InventoryItemInstance = Item;
			AddItem(DataObj);
		}
	}
	
	int32 CurrentCount = GetListItems().Num();
	for (int32 i = CurrentCount; i < Capacity; ++i)
	{
		UMAInventorySlotDataObject* EmptyDataObj = NewObject<UMAInventorySlotDataObject>(this);
		EmptyDataObj->InventoryItemInstance = nullptr; // 비어있음 표시
		AddItem(EmptyDataObj);
	}
}