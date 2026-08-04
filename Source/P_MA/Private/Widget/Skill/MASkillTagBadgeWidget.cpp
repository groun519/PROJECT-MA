#include "Widget/Skill/MASkillTagBadgeWidget.h"

#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Definition/MASkillWarningTextData.h"

static FText ResolveTagLabel(const FGameplayTag& Tag)
{
	FString TagString = Tag.ToString();
	FString UnusedPrefix;
	FString LastName;
	if (TagString.Split(TEXT("."), &UnusedPrefix, &LastName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
	{
		return FText::FromString(LastName);
	}

	return FText::FromString(TagString);
}

static FMASkillTagStyle ResolveTagStyle(const FGameplayTag& Tag, const UDataTable* WarningTextDataTable)
{
	FMASkillTagStyle TagStyle;
	if (!Tag.IsValid() || !WarningTextDataTable) return TagStyle;

	TArray<FMASkillWarningTextDataRow*> TextRows;
	WarningTextDataTable->GetAllRows(TEXT("SkillTagStyleLookup"), TextRows);
	for (const FMASkillWarningTextDataRow* TextRow : TextRows)
	{
		if (!TextRow || TextRow->ReasonTag != Tag) continue;
		TagStyle.BackgroundColor = TextRow->TagBackgroundColor;
		TagStyle.TextColor = TextRow->TagTextColor;
		return TagStyle;
	}

	return TagStyle;
}

void UMASkillTagBadgeWidget::SetTag(const FText& InTagText, const FMASkillTagStyle& InTagStyle)
{
	if (TagText)
	{
		TagText->SetText(InTagText);
		TagText->SetColorAndOpacity(FSlateColor(InTagStyle.TextColor));
	}

	if (BackgroundImage)
	{
		BackgroundImage->SetColorAndOpacity(InTagStyle.BackgroundColor);
	}
}

void UMASkillTagBadgeWidget::RefreshTagBadges(
	UWidget* WidgetContext,
	UPanelWidget* TagBadgePanel,
	TSubclassOf<UMASkillTagBadgeWidget> TagBadgeWidgetClass,
	const FGameplayTagContainer& Tags,
	const UDataTable* WarningTextDataTable)
{
	if (!WidgetContext || !TagBadgePanel) return;

	TagBadgePanel->ClearChildren();

	if (Tags.Num() == 0 || !TagBadgeWidgetClass)
	{
		TagBadgePanel->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	for (auto It = Tags.CreateConstIterator(); It; ++It)
	{
		const FGameplayTag& Tag = *It;
		if (!Tag.IsValid()) continue;

		UMASkillTagBadgeWidget* TagBadgeWidget = CreateWidget<UMASkillTagBadgeWidget>(WidgetContext, TagBadgeWidgetClass);
		if (!TagBadgeWidget) continue;

		TagBadgeWidget->SetTag(ResolveTagLabel(Tag), ResolveTagStyle(Tag, WarningTextDataTable));
		TagBadgePanel->AddChild(TagBadgeWidget);
	}

	TagBadgePanel->SetVisibility(TagBadgePanel->GetChildrenCount() > 0
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
}
