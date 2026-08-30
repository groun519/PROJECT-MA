#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAEnchantmentNodeWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UMAModuleIconWidget;
class UMASkillModule;
class UMASkillModuleInstance;
class UMASkillTooltipWidget;

DECLARE_MULTICAST_DELEGATE(FMAEnchantmentNodeSelectedSignature);

UCLASS()
class P_MA_API UMAEnchantmentNodeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetModule(const UMASkillModule& Module);
	void SetModuleInstance(const UMASkillModuleInstance& ModuleInstance);
	void SetEmpty();
	void SetSelected(bool bSelected);

	FMAEnchantmentNodeSelectedSignature OnSelected;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> NodeButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMAModuleIconWidget> ModuleIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SelectionImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> EmptyText;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

private:
	void SetModuleVisual(const UMASkillModule& Module);

	UFUNCTION()
	void HandleNodeButtonClicked();
};
