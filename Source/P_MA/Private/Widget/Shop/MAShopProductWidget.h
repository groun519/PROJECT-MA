#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/MAShopTypes.h"
#include "MAShopProductWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FMAShopProductSelectedSignature, int32);

UCLASS()
class P_MA_API UMAShopProductWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetProduct(const FMAShopProduct& Product);
	void SetSelected(bool bSelected);
	int32 GetStockId() const { return StockId; }

	FMAShopProductSelectedSignature OnProductSelected;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ProductButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ProductIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> GlowImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PriceText;

private:
	UFUNCTION()
	void HandleProductButtonClicked();

	int32 StockId = INDEX_NONE;
	FLinearColor FrameColor = FLinearColor::White;
};
