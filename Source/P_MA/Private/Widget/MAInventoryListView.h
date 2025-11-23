// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TileView.h"
#include "Inventory/InventoryItem.h"
#include "MAInventoryListView.generated.h"

/**
 * [추가] 리스트 뷰(TileView)의 각 슬롯에 전달될 배달부 객체
 * UInventoryItem은 UObject지만, 리스트 뷰 전용 래퍼가 있으면 관리가 편합니다.
 */
UCLASS(BlueprintType)
class UMAInventorySlotDataObject : public UObject
{
	GENERATED_BODY()

public:
	// 실제 아이템 데이터 인스턴스
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	UInventoryItem* InventoryItemInstance;
};

/**
 * 인벤토리 전용 타일 뷰
 */
UCLASS()
class UMAInventoryListView : public UTileView
{
	GENERATED_BODY()

public:
	// 인벤토리 컴포넌트의 아이템 목록을 받아서 리스트를 갱신하는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateInventoryList(const TArray<UInventoryItem*>& InventoryItems, int32 Capacity);
};