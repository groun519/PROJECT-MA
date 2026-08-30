#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAEnchantmentEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UMAModuleIconWidget;
class UMASkillModule;
class UMASkillModuleInstance;
class UMASkillTooltipWidget;

DECLARE_MULTICAST_DELEGATE(FMAEnchantmentEntrySelectedSignature);

UCLASS()
class P_MA_API UMAEnchantmentEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetModule(const UMASkillModule& Module, int32 Count);
	void SetModuleInstance(const UMASkillModuleInstance& ModuleInstance, const FText& PositionText);
	void SetSelected(bool bSelected);

	FMAEnchantmentEntrySelectedSignature OnSelected;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMAModuleIconWidget> ModuleIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SelectionImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CountText;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

private:
	void SetModuleVisual(const UMASkillModule& Module, const FText& DetailText);

	UFUNCTION()
	void HandleEntryButtonClicked();
};
