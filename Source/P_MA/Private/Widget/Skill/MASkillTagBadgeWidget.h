#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "MASkillTagBadgeWidget.generated.h"

class UDataTable;
class UImage;
class UPanelWidget;
class UTextBlock;
class UWidget;

struct FMASkillTagStyle
{
	FLinearColor BackgroundColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.f);
	FLinearColor TextColor = FLinearColor::White;
};

UCLASS()
class P_MA_API UMASkillTagBadgeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTag(const FText& InTagText, const FMASkillTagStyle& InTagStyle);
	static void RefreshTagBadges(
		UWidget* WidgetContext,
		UPanelWidget* TagBadgePanel,
		TSubclassOf<UMASkillTagBadgeWidget> TagBadgeWidgetClass,
		const FGameplayTagContainer& Tags,
		const UDataTable* WarningTextDataTable);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TagText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> BackgroundImage;
};
