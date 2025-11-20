// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MAAssetManager.h"


UMAAssetManager& UMAAssetManager::Get()
{
	UMAAssetManager* Singleton = Cast<UMAAssetManager>(GEngine->AssetManager.Get());
	if (Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogLoad, Fatal, TEXT("Asset Manager Needs to be of the type CAssetMaanger"));
	return (*NewObject<UMAAssetManager>());
}

void UMAAssetManager::LoadShopItems(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPA_ShopItem::GetShopItemAssetType(), TArray<FName>(), FStreamableDelegate::CreateUObject(this, &UMAAssetManager::ShopItemLoadFinished, LoadFinishedCallback));
}

bool UMAAssetManager::GetLoadedShopItems(TArray<const UPA_ShopItem*>& OutItems) const
{
	TArray<UObject*> LoadedObjects;
	bool bLoaded = GetPrimaryAssetObjectList(UPA_ShopItem::GetShopItemAssetType(), LoadedObjects);

	if (bLoaded)
	{
		for (UObject* ObjectLoaded : LoadedObjects)
		{
			OutItems.Add(Cast<UPA_ShopItem>(ObjectLoaded));
		}
	}

	return bLoaded;
}

const FItemCollection* UMAAssetManager::GetCombinationForItem(const UPA_ShopItem* Item) const
{
	return CombinationMap.Find(Item);
}

const FItemCollection* UMAAssetManager::GetIngredientForItem(const UPA_ShopItem* Item) const
{
	return IngredientMap.Find(Item);
}

void UMAAssetManager::ShopItemLoadFinished(FStreamableDelegate Callback)
{
	Callback.ExecuteIfBound();
	BuildItemMaps();
}

void UMAAssetManager::BuildItemMaps()
{
	TArray<const UPA_ShopItem*> LoadedItems;
	if (GetLoadedShopItems(LoadedItems))
	{
		for (const UPA_ShopItem* Item : LoadedItems)
		{
			if (Item->GetIngredients().Num() == 0)
			{
				continue;
			}

			TArray<const UPA_ShopItem*> Items;
			for (const TSoftObjectPtr<UPA_ShopItem>& Ingredient : Item->GetIngredients())
			{
				UPA_ShopItem* IngredientItem = Ingredient.LoadSynchronous();
				Items.Add(IngredientItem);
				AddToCombinationMap(IngredientItem, Item);
			}

			IngredientMap.Add(Item, FItemCollection{Items});
		}
	}
}

void UMAAssetManager::AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem)
{
	FItemCollection* Combinations = CombinationMap.Find(Ingredient);
	if (Combinations)
	{
		if (!Combinations->Contains(CombinationItem))
			CombinationMap.Add(CombinationItem);
	}
	else
	{
		CombinationMap.Add(Ingredient, FItemCollection{TArray<const UPA_ShopItem*>{CombinationItem}});
	}
}