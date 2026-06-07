#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/MAShopTypes.h"
#include "MAShopDetailWidget.generated.h"

class UImage;
class URichTextBlock;
class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE(FMAShopBuyRequestedSignature);

UCLASS()
class P_MA_API UMAShopDetailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetEntry(const FMAShopStockEntry* InEntry);

	FMAShopBuyRequestedSignature OnBuyRequested;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> QualityText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<URichTextBlock> DescriptionText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PriceText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BuyButton;

private:
	UFUNCTION()
	void HandleBuyButtonClicked();
};
