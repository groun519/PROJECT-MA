#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/MAShopTypes.h"
#include "MAShopWidget.generated.h"

class AMAShopNPC;
class UMAShopDetailWidget;
class UMAShopItemWidget;
class UButton;
class UPanelWidget;

UCLASS()
class P_MA_API UMAShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void InitializeShop(AMAShopNPC* InShopNPC);
	void RefreshStock();

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> ItemContainer;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMAShopDetailWidget> DetailWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(EditDefaultsOnly, Category="Shop")
	TSubclassOf<UMAShopItemWidget> ItemWidgetClass;

private:
	UFUNCTION()
	void HandleCloseButtonClicked();

	void RebuildItems();
	void HandleItemSelected(int32 StockId);
	void HandleBuyRequested();

	UPROPERTY(Transient)
	TObjectPtr<AMAShopNPC> ShopNPC = nullptr;

	int32 SelectedStockId = INDEX_NONE;
};
