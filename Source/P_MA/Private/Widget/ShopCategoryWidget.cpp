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

	// 1. 테이블 이름 가져오기
	FString TableName = InDataTable->GetName();
	FString KoreanTitle = TableName; // 기본값

	// 2. [수정] 이름에 따라 한글로 변환 (하드코딩 방식)
	if (TableName.Contains("Consumables"))
	{
		KoreanTitle = TEXT("소비 아이템");
	}
	else if (TableName.Contains("Equipments"))
	{
		KoreanTitle = TEXT("장비 아이템");
	}
	else if (TableName.Contains("Skills"))
	{
		KoreanTitle = TEXT("스킬");
	}
    
	// 3. 변환된 이름 설정
	CategoryTitleText->SetText(FText::FromString(KoreanTitle));

	// 4. 리스트 채우기 (기존 코드 유지)
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