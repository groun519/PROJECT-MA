#include "Widget/Shop/MAShopDetailWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Display/MADisplayTypes.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "MAMaterialParams.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Skill/MASkillTagBadgeWidget.h"

void UMAShopDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuyButton->OnClicked.RemoveDynamic(this, &UMAShopDetailWidget::HandleBuyButtonClicked);
	BuyButton->OnClicked.AddDynamic(this, &UMAShopDetailWidget::HandleBuyButtonClicked);
}

void UMAShopDetailWidget::SetProduct(const FMAShopProduct* Product)
{
	const UMASkillModule* Module = Product ? Product->Module : nullptr;
	const UMAGameSettings* GameSettings = UMAGameSettings::Get();
	const UMAModuleQualityData* QualityData = GameSettings->GetModuleQualityData();
	const FMADisplayData DisplayData = Module
		? Module->ResolveDisplayData(QualityData)
		: FMADisplayData();
	const FMAIconData& IconData = DisplayData.IconData;
	const ESlateVisibility ProductVisibility = Product ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	const ESlateVisibility BuyVisibility = Product ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	UTexture2D* Icon = IconData.Icon;
	if (UMaterialInstanceDynamic* IconMaterial = ProductIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, Icon);
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

	NameText->SetText(DisplayData.DisplayName);
	NameText->SetVisibility(ProductVisibility);

	const FMAModuleRarityData* RarityData = Module && QualityData
		? QualityData->FindRarityData(Module->GetModuleQuality().Rarity)
		: nullptr;
	QualityText->SetText(RarityData ? RarityData->DisplayName : FText());
	QualityText->SetColorAndOpacity(FSlateColor(IconData.FrameColor));
	QualityText->SetVisibility(RarityData ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	DescriptionText->SetText(DisplayData.Description);
	DescriptionText->SetVisibility(ProductVisibility);

	const float CooldownSeconds = Module ? Module->GetCooldownSeconds() : 0.f;
	FNumberFormattingOptions CooldownFormatting;
	CooldownFormatting.MinimumFractionalDigits = 0;
	CooldownFormatting.MaximumFractionalDigits = FMath::Abs(CooldownSeconds) >= 1.f ? 1 : 2;
	CooldownText->SetText(FText::Format(
		NSLOCTEXT("MAShopDetailWidget", "CooldownSecondsFormat", "{0}s"),
		FText::AsNumber(CooldownSeconds, &CooldownFormatting)));
	CooldownText->SetColorAndOpacity(FSlateColor(CooldownSeconds >= 0.f
		? GameSettings->PositiveCooldownColor
		: GameSettings->NegativeCooldownColor));
	CooldownText->SetVisibility(FMath::IsNearlyZero(CooldownSeconds) ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);

	const UDataTable* WarningTextDataTable = GameSettings->GetWarningTextDataTable();
	const FGameplayTagContainer TooltipTags = Module ? Module->GetTooltipTags() : FGameplayTagContainer();
	UMASkillTagBadgeWidget::RefreshTagBadges(
		this,
		TagBadgePanel,
		TagBadgeWidgetClass,
		TooltipTags,
		WarningTextDataTable);

	PriceText->SetText(Product ? FText::AsNumber(Product->Price) : FText());
	PriceText->SetVisibility(ProductVisibility);

	BuyButton->SetVisibility(BuyVisibility);
}

void UMAShopDetailWidget::HandleBuyButtonClicked()
{
	OnBuyRequested.Broadcast();
}

