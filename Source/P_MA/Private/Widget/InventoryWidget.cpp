// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/InventoryWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Inventory/InventoryComponent.h"
#include "Widget/InventoryItemWidget.h"
#include "Widget/InventoryContextMenuWidget.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		InventoryComponent = OwnerPawn->GetComponentByClass<UInventoryComponent>();
		if (InventoryComponent)
		{
			InventoryComponent->OnItemAdded.AddUObject(this, &UInventoryWidget::ItemAdded);
			InventoryComponent->OnItemRemoved.AddUObject(this, &UInventoryWidget::ItemRemoved);
			InventoryComponent->OnItemStackCountChanged.AddUObject(this, &UInventoryWidget::ItemStackCountChanged);
			InventoryComponent->OnItemAbilityCommitted.AddUObject(this, &UInventoryWidget::ItemAbilityCommitted);
			int Capacity = InventoryComponent->GetCapacity();

			ItemList->ClearChildren();

			for (int i = 0; i < Capacity; ++i)
			{
				UInventoryItemWidget* NewEmptyWidget = CreateWidget<UInventoryItemWidget>(GetOwningPlayer(), ItemWidgetClass);
				if (NewEmptyWidget)
				{
					NewEmptyWidget->SetSlotNumber(i);
					UWrapBoxSlot* NewItemSlot = ItemList->AddChildToWrapBox(NewEmptyWidget);
					NewItemSlot->SetPadding(FMargin(2.f));
					ItemWidgets.Add(NewEmptyWidget);

					NewEmptyWidget->OnInventoryItemDropped.AddUObject(this, &UInventoryWidget::HandleItemDragDrop);
					NewEmptyWidget->OnLeftBttonClicked.AddUObject(InventoryComponent, &UInventoryComponent::TryActivateItem);
					NewEmptyWidget->OnRightBttonClicked.AddUObject(this, &UInventoryWidget::ToggleContextMenu);
				}
			}
			SpawnContextMenu();
		}
	}
}

void UInventoryWidget::NativeOnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusChanging(PreviousFocusPath, NewWidgetPath, InFocusEvent);
	if (!NewWidgetPath.ContainsWidget(ContextMenuWidget->GetCachedWidget().Get()))
	{
		ClearContextMenu();
	}
}

void UInventoryWidget::SpawnContextMenu()
{
	if (!ContextMenuWidgetClass)
		return;

	ContextMenuWidget = CreateWidget<UInventoryContextMenuWidget>(this, ContextMenuWidgetClass);
	if (ContextMenuWidget)
	{
		ContextMenuWidget->GetSellButtonClickedEvent().AddDynamic(this, &UInventoryWidget::SellFocusedItem);
		ContextMenuWidget->GetUseButtonClickedEvent().AddDynamic(this, &UInventoryWidget::UseFocusedItem);
		ContextMenuWidget->AddToViewport(1);
		SetContextMenuVisible(false);
	}
}

void UInventoryWidget::SellFocusedItem()
{
	InventoryComponent->SellItem(CurrentFocusedItemHandle);
	SetContextMenuVisible(false);
}

void UInventoryWidget::UseFocusedItem()
{
	InventoryComponent->TryActivateItem(CurrentFocusedItemHandle);
	SetContextMenuVisible(false);
}

void UInventoryWidget::SetContextMenuVisible(bool bContextMenuVisible)
{
	if (ContextMenuWidget)
	{
		ContextMenuWidget->SetVisibility(bContextMenuVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UInventoryWidget::ToggleContextMenu(const FInventoryItemHandle& ItemHandle)
{
	if (CurrentFocusedItemHandle == ItemHandle)
	{
		ClearContextMenu();
		return;
	}

	CurrentFocusedItemHandle = ItemHandle;
	UInventoryItemWidget** ItemWidgetPtrPtr = PopulatedItemEntryWidgets.Find(ItemHandle);
	if (!ItemWidgetPtrPtr)
		return;

	UInventoryItemWidget* ItemWidget = *ItemWidgetPtrPtr;
	if (!ItemWidget)
		return;

	SetContextMenuVisible(true);
	FVector2D ItemAbsPos = ItemWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D{1.f, 0.5f});

	FVector2D ItemWidgetPixelPos, ItemWidgetViewportPos;
	USlateBlueprintLibrary::AbsoluteToViewport(this, ItemAbsPos, ItemWidgetPixelPos, ItemWidgetViewportPos);

	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (OwningPlayerController)
	{
		int ViewportSizeX, ViewportSizeY;
		OwningPlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
		float Scale = UWidgetLayoutLibrary::GetViewportScale(this);

		int Overshoot = ItemWidgetPixelPos.Y + ContextMenuWidget->GetDesiredSize().Y * Scale - ViewportSizeY;
		if (Overshoot > 0)
		{
			ItemWidgetPixelPos.Y -= Overshoot;
		}
	}

	ContextMenuWidget->SetPositionInViewport(ItemWidgetPixelPos);
}

void UInventoryWidget::ClearContextMenu()
{
	ContextMenuWidget->SetVisibility(ESlateVisibility::Hidden);
	CurrentFocusedItemHandle = FInventoryItemHandle::InvalidHandle();
}


void UInventoryWidget::ItemAdded(const UInventoryItem* InventoryItem)
{
	if (!InventoryItem)
		return;

	if (UInventoryItemWidget* NextAvaliableSlot = GetNextAvaliableSlot())
	{
		NextAvaliableSlot->UpdateInventoryItem(InventoryItem);
		PopulatedItemEntryWidgets.Add(InventoryItem->GetHandle(), NextAvaliableSlot);
		if (InventoryComponent)
		{
			InventoryComponent->ItemSlotChanged(InventoryItem->GetHandle(), NextAvaliableSlot->GetSlotNumber());
		}
	}
}

void UInventoryWidget::ItemStackCountChanged(const FInventoryItemHandle& Handle, int NewCount)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(Handle);
	if (FoundWidget)
	{
		(*FoundWidget)->UpdateStackCount();
	}
}

UInventoryItemWidget* UInventoryWidget::GetNextAvaliableSlot() const
{
	for (UInventoryItemWidget* Widget : ItemWidgets)
	{
		if (Widget->IsEmpty())
		{
			return Widget;
		}
	}

	return nullptr;
}

void UInventoryWidget::HandleItemDragDrop(UInventoryItemWidget* DestinationWidget, UInventoryItemWidget* SourceWidget)
{
	const UInventoryItem* SrcItem = SourceWidget->GetInventoryItem();
	const UInventoryItem* DestinationItem = DestinationWidget->GetInventoryItem();

	DestinationWidget->UpdateInventoryItem(SrcItem);
	SourceWidget->UpdateInventoryItem(DestinationItem);

	PopulatedItemEntryWidgets[DestinationWidget->GetItemHandle()] = DestinationWidget;

	if (InventoryComponent)
	{
		InventoryComponent->ItemSlotChanged(DestinationWidget->GetItemHandle(), DestinationWidget->GetSlotNumber());
	}

	if (!SourceWidget->IsEmpty())
	{
		PopulatedItemEntryWidgets[SourceWidget->GetItemHandle()] = SourceWidget;
		if (InventoryComponent)
		{
			InventoryComponent->ItemSlotChanged(SourceWidget->GetItemHandle(), SourceWidget->GetSlotNumber());
		}
	}
}

void UInventoryWidget::ItemRemoved(const FInventoryItemHandle& ItemHandle)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(ItemHandle);
	if (FoundWidget && *FoundWidget)
	{
		(*FoundWidget)->EmptySlot();
		PopulatedItemEntryWidgets.Remove(ItemHandle);
	}
}

void UInventoryWidget::ItemAbilityCommitted(const FInventoryItemHandle& ItemHandle, float CooldownDuration, float CooldownTimeRemaining)
{
	UInventoryItemWidget** FoundWidget = PopulatedItemEntryWidgets.Find(ItemHandle);
	if (FoundWidget && *FoundWidget)
	{
		(*FoundWidget)->StartCooldown(CooldownDuration, CooldownTimeRemaining);
	}
}