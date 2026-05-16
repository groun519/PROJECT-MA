#include "Widget/Shop/MAShopWidget.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Player/MAPlayerController.h"
#include "Shop/MAShopNPC.h"
#include "Widget/Shop/MAShopDetailWidget.h"
#include "Widget/Shop/MAShopItemWidget.h"

void UMAShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CloseButton->OnClicked.RemoveDynamic(this, &UMAShopWidget::HandleCloseButtonClicked);
	CloseButton->OnClicked.AddDynamic(this, &UMAShopWidget::HandleCloseButtonClicked);

	DetailWidget->OnBuyRequested.RemoveAll(this);
	DetailWidget->OnBuyRequested.AddUObject(this, &UMAShopWidget::HandleBuyRequested);
	DetailWidget->SetEntry(nullptr);
}

void UMAShopWidget::InitializeShop(AMAShopNPC* InShopNPC)
{
	ShopNPC = InShopNPC;
	RebuildItems();
}

void UMAShopWidget::RefreshStock()
{
	RebuildItems();
}

void UMAShopWidget::HandleCloseButtonClicked()
{
	ShopNPC->CloseShop(GetOwningPlayer());
}

void UMAShopWidget::RebuildItems()
{
	ItemContainer->ClearChildren();

	const TArray<FMAShopStockEntry>& StockEntries = ShopNPC->GetCurrentStockEntries();
	for (int32 EntryIndex = 0; EntryIndex < StockEntries.Num(); ++EntryIndex)
	{
		UMAShopItemWidget* ItemWidget = CreateWidget<UMAShopItemWidget>(this, ItemWidgetClass);

		ItemWidget->InitializeItem(StockEntries[EntryIndex]);
		ItemWidget->OnItemSelected.AddUObject(this, &UMAShopWidget::HandleItemSelected);
		ItemContainer->AddChild(ItemWidget);
	}

	if (StockEntries.IsEmpty())
	{
		HandleItemSelected(INDEX_NONE);
		return;
	}

	const bool bSelectionStillExists = StockEntries.ContainsByPredicate([this](const FMAShopStockEntry& Entry)
	{
		return Entry.StockId == SelectedStockId;
	});
	HandleItemSelected(bSelectionStillExists ? SelectedStockId : StockEntries[0].StockId);
}

void UMAShopWidget::HandleItemSelected(int32 StockId)
{
	SelectedStockId = StockId;
	const TArray<FMAShopStockEntry>& StockEntries = ShopNPC->GetCurrentStockEntries();
	const FMAShopStockEntry* Entry = StockEntries.FindByPredicate([StockId](const FMAShopStockEntry& Candidate)
	{
		return Candidate.StockId == StockId;
	});
	DetailWidget->SetEntry(Entry);

	for (int32 ChildIndex = 0; ChildIndex < ItemContainer->GetChildrenCount(); ++ChildIndex)
	{
		if (UMAShopItemWidget* ItemWidget = Cast<UMAShopItemWidget>(ItemContainer->GetChildAt(ChildIndex)))
		{
			ItemWidget->SetSelected(ItemWidget->GetStockId() == SelectedStockId);
		}
	}
}

void UMAShopWidget::HandleBuyRequested()
{
	AMAPlayerController* PlayerController = CastChecked<AMAPlayerController>(GetOwningPlayer());
	PlayerController->RequestShopPurchase(ShopNPC, SelectedStockId);
}
