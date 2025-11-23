// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAInventoryListView.h"

void UMAInventoryListView::UpdateInventoryList(const TArray<UInventoryItem*>& InventoryItems, int32 Capacity)
{
	// 목록 초기화
	ClearListItems();

	// 1. 실제 아이템들 추가
	for (UInventoryItem* Item : InventoryItems)
	{
		if (Item && Item->IsValid())
		{
			UMAInventorySlotDataObject* DataObj = NewObject<UMAInventorySlotDataObject>(this);
			DataObj->InventoryItemInstance = Item;
			AddItem(DataObj);
		}
	}

	// 2. (선택사항) 빈 슬롯도 보여주고 싶다면 아래 로직 사용
	// 인벤토리 용량(Capacity)만큼 모자란 개수를 빈 슬롯으로 채움
	int32 CurrentCount = GetListItems().Num();
	for (int32 i = CurrentCount; i < Capacity; ++i)
	{
		UMAInventorySlotDataObject* EmptyDataObj = NewObject<UMAInventorySlotDataObject>(this);
		EmptyDataObj->InventoryItemInstance = nullptr; // 비어있음 표시
		AddItem(EmptyDataObj);
	}
}