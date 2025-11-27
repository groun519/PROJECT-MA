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

	CategoryContainer->ClearChildren();

	UE_LOG(LogTemp, Warning, TEXT("[ShopWidget] Creating Categories from %d Tables"), ShopDataTables.Num());
	
	for (UDataTable* Table : ShopDataTables)
	{
		if (!Table) continue;
		
		UShopCategoryWidget* NewCategory = CreateWidget<UShopCategoryWidget>(this, CategoryWidgetClass);
		
		if (NewCategory)
		{
			NewCategory->InitCategory(Table);
			
			NewCategory->OnCategoryPurchaseRequested.AddDynamic(this, &UShopWidget::OnPurchaseRequested);

			CategoryContainer->AddChild(NewCategory);
		}
	}
}

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