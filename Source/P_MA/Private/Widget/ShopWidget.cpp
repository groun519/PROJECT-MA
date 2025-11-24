#include "Widget/ShopWidget.h"
#include "Widget/ShopCategoryWidget.h"
#include "Widget/ShopItemWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Engine/DataTable.h"
#include "Inventory/MAItemTypes.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);

	if (APawn* OwnerPawn = GetOwningPlayerPawn())
	{
		OwnerInventoryComponent = OwnerPawn->GetComponentByClass<UInventoryComponent>();
	}

	// 바로 로드 시도
	LoadShopCategories();
}

void UShopWidget::InitShop(const TArray<UDataTable*>& InDataTables)
{
	ShopDataTables = InDataTables;
	LoadShopCategories();
}

void UShopWidget::LoadShopCategories()
{
	if (!CategoryContainer || !CategoryWidgetClass) return;

	CategoryContainer->ClearChildren(); // 기존 카테고리 싹 지움

	UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] Creating Categories from %d Tables"), ShopDataTables.Num());

	// 테이블 개수만큼 카테고리 위젯 생성 (소비, 장비, 스킬 등)
	for (UDataTable* Table : ShopDataTables)
	{
		if (!Table) continue;

		// 1. 카테고리 위젯 생성
		UShopCategoryWidget* NewCategory = CreateWidget<UShopCategoryWidget>(this, CategoryWidgetClass);
		
		if (NewCategory)
		{
			// 2. 초기화 (내부 TileView 채우기)
			NewCategory->InitCategory(Table);

			// 3. 구매 이벤트 연결 (카테고리 -> 샵)
			NewCategory->OnCategoryPurchaseRequested.AddDynamic(this, &UShopWidget::OnPurchaseRequested);

			// 4. 스크롤 박스에 추가
			CategoryContainer->AddChild(NewCategory);
		}
	}
}

// 구매 로직은 그대로 유지
void UShopWidget::OnPurchaseRequested(const UShopItemDataObject* ItemDataObject)
{
	if (OwnerInventoryComponent && ItemDataObject && ItemDataObject->CachedItemData)
	{
		if (ItemDataObject->CachedItemData->ItemType == EMAItemType::Skill)
		{
			OwnerInventoryComponent->TryPurchaseSkill(ItemDataObject->ItemRowName, ItemDataObject->SourceDataTable);
		}
		else
		{
			OwnerInventoryComponent->TryPurchaseItem(ItemDataObject->ItemRowName, ItemDataObject->SourceDataTable);
		}
	}
}