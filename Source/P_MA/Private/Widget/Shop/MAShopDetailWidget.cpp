#include "Widget/Shop/MAShopDetailWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "MAMaterialParams.h"

void UMAShopDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuyButton->OnClicked.RemoveDynamic(this, &UMAShopDetailWidget::HandleBuyButtonClicked);
	BuyButton->OnClicked.AddDynamic(this, &UMAShopDetailWidget::HandleBuyButtonClicked);
}

void UMAShopDetailWidget::SetEntry(const FMAShopStockEntry* InEntry)
{
	const ESlateVisibility EntryVisibility = InEntry ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	const ESlateVisibility BuyVisibility = InEntry ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	const FMASkillDefinitionIconData* IconData = InEntry && InEntry->SkillDefinition ? &InEntry->SkillDefinition->GetDisplayData().IconData : nullptr;
	UTexture2D* Icon = IconData ? IconData->Icon : nullptr;
	if (UMaterialInstanceDynamic* IconMaterial = ItemIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, Icon);
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

	NameText->SetText(InEntry && InEntry->SkillDefinition ? InEntry->SkillDefinition->GetDisplayData().DisplayName : FText());
	NameText->SetVisibility(EntryVisibility);

	DescriptionText->SetText(InEntry && InEntry->SkillDefinition ? InEntry->SkillDefinition->GetDisplayData().Description : FText());
	DescriptionText->SetVisibility(EntryVisibility);

	PriceText->SetText(InEntry ? FText::AsNumber(InEntry->Price) : FText());
	PriceText->SetVisibility(EntryVisibility);

	BuyButton->SetVisibility(BuyVisibility);
}

void UMAShopDetailWidget::HandleBuyButtonClicked()
{
	OnBuyRequested.Broadcast();
}

