#include "Widget/Shop/MAShopProductWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Display/MADisplayTypes.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "MAMaterialParams.h"
#include "Setting/MAGameSettings.h"

static const FLinearColor SelectedFrameColor = FLinearColor(30.f, 0.75f, 0.75f, 1.f).HSVToLinearRGB();
static constexpr float ProductTranslationRange = 20.f;
static constexpr float IconAngleRange = 10.f;

void UMAShopProductWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ProductButton->OnClicked.RemoveDynamic(this, &UMAShopProductWidget::HandleProductButtonClicked);
	ProductButton->OnClicked.AddDynamic(this, &UMAShopProductWidget::HandleProductButtonClicked);
}

void UMAShopProductWidget::SetProduct(const FMAShopProduct& Product)
{
	StockId = Product.StockId;
	const UMASkillModule* Module = Product.Module;
	const UMAModuleQualityData* QualityData = UMAGameSettings::Get()->GetModuleQualityData();
	const FMAIconData IconData = Module
		? Module->ResolveDisplayData(QualityData).IconData
		: FMAIconData();
	FrameColor = IconData.FrameColor;

	FRandomStream VisualRandom(Product.VisualSeed);
	SetRenderTranslation(FVector2D(
		VisualRandom.FRandRange(-ProductTranslationRange, ProductTranslationRange),
		VisualRandom.FRandRange(-ProductTranslationRange, ProductTranslationRange)));
	ProductIconImage->SetRenderTransformAngle(VisualRandom.FRandRange(-IconAngleRange, IconAngleRange));

	UTexture2D* Icon = IconData.Icon;
	if (UMaterialInstanceDynamic* IconMaterial = ProductIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, Icon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, FrameColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		ProductIconImage->SetVisibility(Icon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	else if (Icon)
	{
		ProductIconImage->SetBrushFromTexture(Icon);
		ProductIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		ProductIconImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	const FMAModuleRarityData* RarityData = Module && QualityData
		? QualityData->FindRarityData(Module->GetModuleQuality().Rarity)
		: nullptr;
	const float GlowAlpha = RarityData ? RarityData->GlowAlpha : 0.f;
	if (UMaterialInstanceDynamic* GlowMaterial = GlowImage->GetDynamicMaterial())
	{
		GlowMaterial->SetVectorParameterValue(PARAM_ShopGlow_BaseColor, FrameColor);
		GlowMaterial->SetScalarParameterValue(PARAM_ShopGlow_Alpha, GlowAlpha);
	}
	GlowImage->SetVisibility(GlowAlpha > 0.f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	PriceText->SetText(FText::AsNumber(Product.Price));
	PriceText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMAShopProductWidget::SetSelected(bool bSelected)
{
	if (UMaterialInstanceDynamic* IconMaterial = ProductIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, bSelected ? SelectedFrameColor : FrameColor);
	}
}

void UMAShopProductWidget::HandleProductButtonClicked()
{
	if (StockId == INDEX_NONE) return;
	OnProductSelected.Broadcast(StockId);
}
