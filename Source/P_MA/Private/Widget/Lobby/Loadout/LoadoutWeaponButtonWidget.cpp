// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutWeaponButtonWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"

void ULoadoutWeaponButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EquippedBorder)
	{
		EquippedBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		EquippedBorder->SetOpacity(0.0f);
	}

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

	if (EquippedBorder)
	{
		EquippedBorder->SetOpacity(bSelected ? 1.0f : 0.0f);
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

	WeaponButton->SetStyle(BaseStyle);
}
