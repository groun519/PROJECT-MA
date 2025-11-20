// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/InventoryItemDragDropOp.h"
#include "Widget/InventoryItemWidget.h"
#include "Widget/ItemWidget.h"

void UInventoryItemDragDropOp::SetDraggedItem(UInventoryItemWidget* DraggedItem)
{
	Payload = DraggedItem;
	if (DragVisualClass)
	{
		UItemWidget* DragItemWidget = CreateWidget<UItemWidget>(GetWorld(), DragVisualClass);
		if (DragItemWidget)
		{
			DragItemWidget->SetIcon(DraggedItem->GetIconTexture());
			DefaultDragVisual = DragItemWidget;
		}
	}
}