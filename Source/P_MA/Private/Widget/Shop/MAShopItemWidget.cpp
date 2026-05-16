#include "Widget/Shop/MAShopItemWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "MAMaterialParams.h"

static const FLinearColor DefaultFrameColor = FLinearColor(0.f, 0.f, 0.5f, 1.f).HSVToLinearRGB();
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

	FRandomStream VisualRandom(InEntry.VisualSeed);
	SetRenderTranslation(FVector2D(
		VisualRandom.FRandRange(-ItemTranslationRange, ItemTranslationRange),
		VisualRandom.FRandRange(-ItemTranslationRange, ItemTranslationRange)));
	ItemIconImage->SetRenderTransformAngle(VisualRandom.FRandRange(-IconAngleRange, IconAngleRange));

	const FMASkillDefinitionIconData* IconData = InEntry.SkillDefinition ? &InEntry.SkillDefinition->GetDisplayData().IconData : nullptr;
	UTexture2D* Icon = IconData ? IconData->Icon : nullptr;
	if (UMaterialInstanceDynamic* IconMaterial = ItemIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, Icon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, DefaultFrameColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData ? IconData->IconColor : FLinearColor::White);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData ? IconData->InnerColor : FLinearColor(0.15f, 0.15f, 0.15f, 1.f));
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

	PriceText->SetText(FText::AsNumber(InEntry.Price));
	PriceText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UMAShopItemWidget::SetSelected(bool bSelected)
{
	if (UMaterialInstanceDynamic* IconMaterial = ItemIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, bSelected ? SelectedFrameColor : DefaultFrameColor);
	}
}

void UMAShopItemWidget::HandleItemButtonClicked()
{
	if (StockId == INDEX_NONE) return;
	OnItemSelected.Broadcast(StockId);
}

