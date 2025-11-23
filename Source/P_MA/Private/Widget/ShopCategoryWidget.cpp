// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/ShopCategoryWidget.h"
#include "Components/TileView.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"
#include "Widget/ShopItemWidget.h"
#include "Inventory/MAItemTypes.h"

void UShopCategoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CategoryItemList)
	{
		CategoryItemList->OnEntryWidgetGenerated().AddUObject(this, &UShopCategoryWidget::HandleItemGenerated);
	}
}

void UShopCategoryWidget::InitCategory(UDataTable* InDataTable)
{
	if (!InDataTable || !CategoryItemList || !CategoryTitleText) return;
	
	FString TableName = InDataTable->GetName();
	TableName.RemoveFromStart("DT_"); 
	CategoryTitleText->SetText(FText::FromString(TableName));
	
	CategoryItemList->ClearListItems();
	
	for (const auto& RowPair : InDataTable->GetRowMap())
	{
		FName RowName = RowPair.Key;
		FBaseItemData* BaseData = reinterpret_cast<FBaseItemData*>(RowPair.Value);

		if (BaseData)
		{
			UShopItemDataObject* DataObj = NewObject<UShopItemDataObject>(this);
			DataObj->ItemRowName = RowName;
			DataObj->SourceDataTable = InDataTable;
			DataObj->CachedItemData = BaseData;

			CategoryItemList->AddItem(DataObj);
		}
	}
}

void UShopCategoryWidget::HandleItemGenerated(UUserWidget& NewWidget)
{
	UShopItemWidget* ItemWidget = Cast<UShopItemWidget>(&NewWidget);
	
	if (ItemWidget)
	{
		if (!ItemWidget->OnItemPurchaseIssued.IsAlreadyBound(this, &UShopCategoryWidget::RelayPurchaseRequest))
		{
			ItemWidget->OnItemPurchaseIssued.AddDynamic(this, &UShopCategoryWidget::RelayPurchaseRequest);
		}
	}
}

void UShopCategoryWidget::RelayPurchaseRequest(const UShopItemDataObject* ItemDataObj)
{
	OnCategoryPurchaseRequested.Broadcast(ItemDataObj);
}