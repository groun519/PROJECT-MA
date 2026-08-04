#include "Widget/Shop/MAShopItemWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "MAMaterialParams.h"

static const FLinearColor SelectedFrameColor = FLinearColor(30.f, 0.75f, 0.75f, 1.f).HSVToLinearRGB();
static constexpr float ItemTranslationRange = 20.f;
static constexpr float IconAngleRange = 10.f;

void UMAShopItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ItemButton->OnClicked.RemoveDynamic(this, &UMAShopItemWidget::HandleItemButtonClicked);
	ItemButton->OnClicked.AddDynamic(this, &UMAShopItemWidget::HandleItemButtonClicked);
}

void UMAShopItemWidget::InitializeItem(const FMAShopStockEntry& InEntry)
{
	StockId = InEntry.StockId;
	FrameColor = InEntry.QualityColor;

	FRandomStream VisualRandom(InEntry.VisualSeed);
	SetRenderTranslation(FVector2D(
		VisualRandom.FRandRange(-ItemTranslationRange, ItemTranslationRange),
		VisualRandom.FRandRange(-ItemTranslationRange, ItemTranslationRange)));
	ItemIconImage->SetRenderTransformAngle(VisualRandom.FRandRange(-IconAngleRange, IconAngleRange));

	UTexture2D* Icon = InEntry.Icon;
	if (UMaterialInstanceDynamic* IconMaterial = ItemIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, Icon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, FrameColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, InEntry.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, InEntry.InnerColor);
		ItemIconImage->SetVisibility(Icon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	else if (Icon)
	{
		ItemIconImage->SetBrushFromTexture(Icon);
		ItemIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		ItemIconImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (UMaterialInstanceDynamic* GlowMaterial = GlowImage->GetDynamicMaterial())
	{
		GlowMaterial->SetVectorParameterValue(PARAM_ShopGlow_BaseColor, InEntry.QualityColor);
		GlowMaterial->SetScalarParameterValue(PARAM_ShopGlow_Alpha, InEntry.GlowAlpha);
	}
	GlowImage->SetVisibility(InEntry.GlowAlpha > 0.f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	PriceText->SetText(FText::AsNumber(InEntry.Price));
	PriceText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMAShopItemWidget::SetSelected(bool bSelected)
{
	if (UMaterialInstanceDynamic* IconMaterial = ItemIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, bSelected ? SelectedFrameColor : FrameColor);
	}
}

void UMAShopItemWidget::HandleItemButtonClicked()
{
	if (StockId == INDEX_NONE) return;
	OnItemSelected.Broadcast(StockId);
}

