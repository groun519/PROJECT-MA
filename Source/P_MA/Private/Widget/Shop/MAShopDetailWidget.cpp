#include "Widget/Shop/MAShopDetailWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
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

void UMAShopDetailWidget::SetEntry(const FMAShopStockEntry* InEntry)
{
	const UMASkillModule* SkillModule = InEntry ? InEntry->SkillModule : nullptr;
	const ESlateVisibility EntryVisibility = InEntry ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	const ESlateVisibility BuyVisibility = InEntry ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	UTexture2D* Icon = InEntry ? InEntry->Icon : nullptr;
	if (UMaterialInstanceDynamic* IconMaterial = ItemIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, Icon);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, InEntry ? InEntry->IconColor : FLinearColor::White);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, InEntry ? InEntry->InnerColor : FLinearColor(0.15f, 0.15f, 0.15f, 1.f));
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

	NameText->SetText(SkillModule ? SkillModule->GetDisplayData().DisplayName : FText());
	NameText->SetVisibility(EntryVisibility);

	QualityText->SetText(InEntry ? InEntry->QualityText : FText());
	QualityText->SetColorAndOpacity(FSlateColor(InEntry ? InEntry->QualityColor : FLinearColor::White));
	QualityText->SetVisibility(InEntry && !InEntry->QualityText.IsEmpty() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	DescriptionText->SetText(SkillModule ? SkillModule->GetDisplayData().Description : FText());
	DescriptionText->SetVisibility(EntryVisibility);

	const float CooldownSeconds = SkillModule ? SkillModule->GetCooldownSeconds() : 0.f;
	FNumberFormattingOptions CooldownFormatting;
	CooldownFormatting.MinimumFractionalDigits = 0;
	CooldownFormatting.MaximumFractionalDigits = FMath::Abs(CooldownSeconds) >= 1.f ? 1 : 2;
	CooldownText->SetText(FText::Format(
		NSLOCTEXT("MAShopDetailWidget", "CooldownSecondsFormat", "{0}s"),
		FText::AsNumber(CooldownSeconds, &CooldownFormatting)));
	const UMAGameSettings* GameSettings = UMAGameSettings::Get();
	CooldownText->SetColorAndOpacity(FSlateColor(CooldownSeconds >= 0.f
		? GameSettings->PositiveCooldownColor
		: GameSettings->NegativeCooldownColor));
	CooldownText->SetVisibility(FMath::IsNearlyZero(CooldownSeconds) ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);

	const UDataTable* WarningTextDataTable = GameSettings->GetWarningTextDataTable();
	const FGameplayTagContainer TooltipTags = SkillModule ? SkillModule->GetTooltipTags() : FGameplayTagContainer();
	UMASkillTagBadgeWidget::RefreshTagBadges(
		this,
		TagBadgePanel,
		TagBadgeWidgetClass,
		TooltipTags,
		WarningTextDataTable);

	PriceText->SetText(InEntry ? FText::AsNumber(InEntry->Price) : FText());
	PriceText->SetVisibility(EntryVisibility);

	BuyButton->SetVisibility(BuyVisibility);
}

void UMAShopDetailWidget::HandleBuyButtonClicked()
{
	OnBuyRequested.Broadcast();
}

