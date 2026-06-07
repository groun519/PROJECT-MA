#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/MAShopTypes.h"
#include "MAShopItemWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FMAShopItemSelectedSignature, int32);

UCLASS()
class P_MA_API UMAShopItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void InitializeItem(const FMAShopStockEntry& InEntry);
	void SetSelected(bool bSelected);
	int32 GetStockId() const { return StockId; }

	FMAShopItemSelectedSignature OnItemSelected;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ItemButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> GlowImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PriceText;

private:
	UFUNCTION()
	void HandleItemButtonClicked();

	int32 StockId = INDEX_NONE;
	FLinearColor FrameColor = FLinearColor::White;
};
