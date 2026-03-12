// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Lobby/Loadout/LoadoutIconButtonWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Materials/MaterialInterface.h"
#include "Styling/SlateBrush.h"

void ULoadoutIconButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EquippedBorder)
	{
		EquippedBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		EquippedBorder->SetOpacity(0.0f);
	}

	if (IconButton)
	{
		IconButton->OnClicked.AddUniqueDynamic(this, &ULoadoutIconButtonWidget::HandleButtonClicked);
	}

	UTexture2D* Texture = IconTexture.LoadSynchronous();
	if (Texture)
	{
		ApplyButtonIcon(
			Texture,
			FVector2D(static_cast<float>(Texture->GetSizeX()), static_cast<float>(Texture->GetSizeY()))
		);
	}
	else if (UMaterialInterface* Material = IconMaterial.LoadSynchronous())
	{
		FVector2D DefaultSize(64.f, 64.f);
		if (IconButton)
		{
			const FVector2D ExistingSize = IconButton->GetStyle().Normal.ImageSize;
			if (ExistingSize.X > 0.f && ExistingSize.Y > 0.f)
			{
				DefaultSize = ExistingSize;
			}
		}

		ApplyButtonIcon(Material, DefaultSize);
	}

	if (IconButton)
	{
		BaseStyle = IconButton->GetStyle();
		ApplyBaseStyle();
	}
}

void ULoadoutIconButtonWidget::OnButtonClicked()
{
}

void ULoadoutIconButtonWidget::HandleButtonClicked()
{
	OnButtonClicked();
}

void ULoadoutIconButtonWidget::SetSelected(bool bInSelected)
{
	if (bSelected == bInSelected)
	{
		return;
	}

	bSelected = bInSelected;
	ApplyBaseStyle();

	if (EquippedBorder)
	{
		EquippedBorder->SetOpacity(bSelected ? 1.0f : 0.0f);
	}
}

void ULoadoutIconButtonWidget::ApplyButtonIcon(UObject* ResourceObject, const FVector2D& ImageSize)
{
	if (!IconButton || !ResourceObject)
	{
		return;
	}

	FButtonStyle Style = IconButton->GetStyle();

	FSlateBrush Brush;
	Brush.SetResourceObject(ResourceObject);
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = ImageSize;

	Style.SetNormal(Brush);
	Style.SetHovered(Brush);
	Style.SetPressed(Brush);
	IconButton->SetStyle(Style);
}

void ULoadoutIconButtonWidget::ApplyBaseStyle()
{
	if (!IconButton)
	{
		return;
	}

	IconButton->SetStyle(BaseStyle);
}
