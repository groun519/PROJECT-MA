// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TileView.h"
#include "Inventory/InventoryItem.h"
#include "MAInventoryListView.generated.h"

/**
 * 
 * 
 */
UCLASS(BlueprintType)
class UMAInventorySlotDataObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	UInventoryItem* InventoryItemInstance;
};

/**
 * 
 */
UCLASS()
class UMAInventoryListView : public UTileView
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateInventoryList(const TArray<UInventoryItem*>& InventoryItems, int32 Capacity);
};