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

	// 1. 제목 설정
	FString TableName = InDataTable->GetName();
	TableName.RemoveFromStart("DT_"); 
	CategoryTitleText->SetText(FText::FromString(TableName));

	// 2. 리스트 채우기
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

// [!!!수정!!!] 파라미터가 포인터(*)로 변경됨
void UShopCategoryWidget::HandleItemGenerated(UUserWidget& NewWidget)
{
	// NewWidget은 참조이므로 주소(&)를 가져와서 캐스팅해야 합니다.
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