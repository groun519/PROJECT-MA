// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Widget/ItemWidget.h"
#include "Inventory/MAItemTypes.h" 
#include "ShopItemWidget.generated.h"

class UShopItemWidget;
class UDataTable;

/**
 * 
 * 
 */
UCLASS(BlueprintType)
class UShopItemDataObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	FName ItemRowName;

	UPROPERTY(BlueprintReadOnly)
	UDataTable* SourceDataTable;
	
	const FBaseItemData* CachedItemData = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPurchaseIssused, const UShopItemDataObject*, ItemDataObj);

UCLASS()
class UShopItemWidget : public UItemWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	FOnItemPurchaseIssused OnItemPurchaseIssued;
  
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	const UShopItemDataObject* GetItemDataObject() const { return ItemDataObject; }

private:
	UPROPERTY()
	const UShopItemDataObject* ItemDataObject;
  	
	virtual void RightButtonClicked() override;
	virtual void LeftButtonClicked() override;
};