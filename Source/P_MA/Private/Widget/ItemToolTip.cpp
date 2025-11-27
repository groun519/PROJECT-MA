// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/ItemToolTip.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UItemToolTip::SetItemData(const FBaseItemData* ItemData)
{
	if (!ItemData) return;
	
	ItemTitleText->SetText(ItemData->DisplayName);
	ItemDescriptionText->SetText(ItemData->Description);
	ItemPriceText->SetText(FText::AsNumber((int)ItemData->Price));
	
	UTexture2D* Texture = ItemData->Icon.LoadSynchronous();
	if (Texture)
	{
		IconImage->SetBrushFromTexture(Texture);
		IconImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UItemToolTip::SetPrice(float newPrice)
{
	ItemPriceText->SetText(FText::AsNumber((int)newPrice));
}