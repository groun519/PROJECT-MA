// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Inventory/PA_ShopItem.h"
#include "MAAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class UMAAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static UMAAssetManager& Get();
	void LoadShopItems(const FStreamableDelegate& LoadFinishedCallback);
	bool GetLoadedShopItems(TArray<const UPA_ShopItem*>& OutItems) const;
private:
	void ShopItemLoadFinished(FStreamableDelegate Callback);
	void BuildItemMaps();
	void AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem);

	UPROPERTY()
	TMap<const UPA_ShopItem*, FItemCollection> CombinationMap;
	UPROPERTY()
	TMap<const UPA_ShopItem*, FItemCollection> IngredientMap;
};