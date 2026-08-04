#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop/MAShopTypes.h"
#include "MAShopWidget.generated.h"

class AMAShopNPC;
struct FOnAttributeChangeData;
class UMAShopDetailWidget;
class UMAShopItemWidget;
class UAbilitySystemComponent;
class UButton;
class UPanelWidget;
class UTextBlock;

UCLASS()
class P_MA_API UMAShopWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void InitializeShop(AMAShopNPC* InShopNPC);
	void RefreshStock();

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UPanelWidget> ItemContainer;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMAShopDetailWidget> DetailWidget;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CoinText;

	UPROPERTY(EditDefaultsOnly, Category="Shop")
	TSubclassOf<UMAShopItemWidget> ItemWidgetClass;

private:
	UFUNCTION()
	void HandleCloseButtonClicked();

	void BindCoinAttributeChanged();
	void UnbindCoinAttributeChanged();
	void RefreshCoinText();
	void RebuildItems();
	void HandleItemSelected(int32 StockId);
	void HandleBuyRequested();

	void HandleCoinAttributeChanged(const FOnAttributeChangeData& ChangeData);

	UPROPERTY(Transient)
	TObjectPtr<AMAShopNPC> ShopNPC = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> BoundCoinAbilitySystemComponent = nullptr;

	FDelegateHandle CoinAttributeChangedHandle;
	int32 SelectedStockId = INDEX_NONE;
	int32 PendingSelectionIndex = INDEX_NONE;
};
