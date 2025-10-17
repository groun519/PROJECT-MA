// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/ItemWidget.h"
#include "Widget/ItemToolTip.h"
#include "Components/Image.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

void UItemWidget::SetIcon(UTexture2D* IconTexture)
{
	ItemIcon->SetBrushFromTexture(IconTexture);
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply SuperReply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget());
	}

	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().SetUserFocus(TakeWidget()).DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return SuperReply;
}

FReply UItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply SuperReply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (HasAnyUserFocus())
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			RightButtonClicked();
			return FReply::Handled();
		}
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			LeftButtonClicked();
			return FReply::Handled();
		}
	}

	return SuperReply;
}

void UItemWidget::RightButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Right mouse button clicked"));
}

void UItemWidget::LeftButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Left mouse button clicked"));
}

UItemToolTip* UItemWidget::SetToolTipWidget(const UPA_ShopItem* Item)
{
	if (!Item)
		return nullptr;

	if (GetOwningPlayer() && ItemToolTipClass)
	{
		UItemToolTip* ToolTip = CreateWidget<UItemToolTip>(GetOwningPlayer(), ItemToolTipClass);
		if (ToolTip)
		{
			ToolTip->SetItem(Item);
			SetToolTip(ToolTip);
		}

		return ToolTip;
	}

	return nullptr;
}

