// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryItem.h"
#include "InventoryWidget.generated.h"

class UInventoryItemWidget;
class UInventoryContextMenuWidget;
/**
 * 
 */
UCLASS()
class UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventoryContextMenuWidget> ContextMenuWidgetClass;

	UPROPERTY()
	UInventoryContextMenuWidget* ContextMenuWidget;

	void SpawnContextMenu();

	UFUNCTION()
	void SellFocusedItem();

	UFUNCTION()
	void UseFocusedItem();

	void SetContextMenuVisible(bool bContextMenuVisible);
	void ToggleContextMenu(const FInventoryItemHandle& ItemHandle);
	void ClearContextMenu();
	FInventoryItemHandle CurrentFocusedItemHandle;
	
	UPROPERTY(meta=(BindWidget))
	class UWrapBox* ItemList;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventoryItemWidget> ItemWidgetClass;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	TArray<UInventoryItemWidget*> ItemWidgets;
	TMap<FInventoryItemHandle, UInventoryItemWidget*> PopulatedItemEntryWidgets;

	void ItemAdded(const UInventoryItem* InventoryItem);
	void ItemStackCountChanged(const FInventoryItemHandle& Handle, int NewCount);

	UInventoryItemWidget* GetNextAvaliableSlot() const;

	void HandleItemDragDrop(UInventoryItemWidget* DestinationWidget, UInventoryItemWidget* SourceWidget);
	void ItemRemoved(const FInventoryItemHandle& ItemHandle);
	void ItemAbilityCommitted(const FInventoryItemHandle& ItemHandle, float CooldownDuration, float CooldownTimeRemaining);
};