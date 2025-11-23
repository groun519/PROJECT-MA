// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopCategoryWidget.generated.h"

class UTileView;
class UTextBlock;
class UDataTable;
class UShopItemDataObject;
class UShopItemWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategoryPurchaseRequested, const UShopItemDataObject*, ItemDataObj);

UCLASS()
class UShopCategoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	void InitCategory(UDataTable* InDataTable);
	
	FOnCategoryPurchaseRequested OnCategoryPurchaseRequested;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CategoryTitleText;

	UPROPERTY(meta = (BindWidget))
	UTileView* CategoryItemList;
	
	void HandleItemGenerated(UUserWidget& NewWidget);

	UFUNCTION()
	void RelayPurchaseRequest(const UShopItemDataObject* ItemDataObj);
};