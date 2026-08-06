#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/MAShopTypes.h"
#include "MAShopDetailWidget.generated.h"

class UImage;
class UPanelWidget;
class URichTextBlock;
class UButton;
class UTextBlock;
class UMASkillTagBadgeWidget;

DECLARE_MULTICAST_DELEGATE(FMAShopBuyRequestedSignature);

UCLASS()
class P_MA_API UMAShopDetailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetProduct(const FMAShopProduct* Product);

	FMAShopBuyRequestedSignature OnBuyRequested;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ProductIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> QualityText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> TagBadgePanel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PriceText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BuyButton;

	UPROPERTY(EditDefaultsOnly, Category="Shop")
	TSubclassOf<UMASkillTagBadgeWidget> TagBadgeWidgetClass;

private:
	UFUNCTION()
	void HandleBuyButtonClicked();
};
