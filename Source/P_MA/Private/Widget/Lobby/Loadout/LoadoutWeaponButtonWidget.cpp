// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWeaponButtonWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"

void ULoadoutWeaponButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WeaponButton)
	{
		WeaponButton->OnClicked.AddDynamic(this, &ULoadoutWeaponButtonWidget::HandleWeaponClicked);
	}

	UTexture2D* Texture = IconTexture.LoadSynchronous();
	if (Texture)
	{
		ApplyButtonIcon(Texture);
	}

	if (WeaponButton)
	{
		BaseStyle = WeaponButton->GetStyle();
		ApplySelectedStyle();
	}
}

void ULoadoutWeaponButtonWidget::HandleWeaponClicked()
{
	OnWeaponSelected.Broadcast(WeaponId);
}

void ULoadoutWeaponButtonWidget::SetSelected(bool bInSelected)
{
	if (bSelected == bInSelected)
	{
		return;
	}

	bSelected = bInSelected;
	ApplySelectedStyle();
}

void ULoadoutWeaponButtonWidget::SetEquipped(bool bInEquipped)
{
	if (bEquipped == bInEquipped)
	{
		return;
	}

	bEquipped = bInEquipped;
	if (WeaponButton)
	{
		WeaponButton->SetIsEnabled(!bEquipped);
	}

	if (EquippedBorder)
	{
		EquippedBorder->SetVisibility(bEquipped ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void ULoadoutWeaponButtonWidget::ApplyButtonIcon(UTexture2D* Texture)
{
	if (!WeaponButton || !Texture)
	{
		return;
	}

	FButtonStyle Style = WeaponButton->GetStyle();

	FSlateBrush Brush;
	Brush.SetResourceObject(Texture);
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = FVector2D(
		static_cast<float>(Texture->GetSizeX()),
		static_cast<float>(Texture->GetSizeY())
	);

	Style.SetNormal(Brush);
	Style.SetHovered(Brush);
	Style.SetPressed(Brush);
	WeaponButton->SetStyle(Style);
}

void ULoadoutWeaponButtonWidget::ApplySelectedStyle()
{
	if (!WeaponButton)
	{
		return;
	}

	FButtonStyle Style = BaseStyle;
	if (bSelected)
	{
		const FLinearColor SelectedTint(0.7f, 1.0f, 0.4f, 1.0f);
		Style.Normal.TintColor = FSlateColor(SelectedTint);
		Style.Hovered.TintColor = FSlateColor(SelectedTint);
		Style.Pressed.TintColor = FSlateColor(SelectedTint);
	}

	WeaponButton->SetStyle(Style);
}
