// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/InventoryContextMenuWidget.h"

FOnButtonClickedEvent& UInventoryContextMenuWidget::GetSellButtonClickedEvent() const
{
	return SellButton->OnClicked;
}

FOnButtonClickedEvent& UInventoryContextMenuWidget::GetUseButtonClickedEvent() const
{
	return UseButton->OnClicked;
}