#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "MASkillIconWidget.generated.h"

class UImage;
class UTextBlock;
class UMASkillDefinition;
class UMASkillTooltipWidget;

UCLASS()
class P_MA_API UMASkillIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHotkeyText(const FText& InText);
	void SetSkillDefinition(const UMASkillDefinition* SkillDefinition);
	void SetCooldownTag(FGameplayTag InCooldownTag);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SkillIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CooldownOverlayImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HotkeyText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

private:
	void RefreshTooltip(const UMASkillDefinition* SkillDefinition);
	void RefreshCooldown();
	void SetCooldownDisplay(float RemainingSeconds, float DurationSeconds);

	UPROPERTY(Transient)
	FGameplayTag CooldownTag;
};
