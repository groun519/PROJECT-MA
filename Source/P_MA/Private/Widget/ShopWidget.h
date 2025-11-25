// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/MovableWindowWidget.h"
#include "Inventory/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Widget/ShopCategoryWidget.h"
#include "ShopWidget.generated.h"

class UTileView;
class UShopItemWidget;
class UDataTable; 
class UShopItemDataObject; 

UCLASS()
class UShopWidget : public UMovableWindowWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitShop(const TArray<UDataTable*>& InDataTables);

private:
	// [변경] TileView 삭제 -> 카테고리들을 담을 스크롤 박스 추가
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* CategoryContainer; 

	// [추가] 생성할 카테고리 위젯 클래스 (WBP_ShopCategory)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShopCategoryWidget> CategoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"))
	TArray<UDataTable*> ShopDataTables;

	UPROPERTY()
	UInventoryComponent* OwnerInventoryComponent;

	void LoadShopCategories(); // 함수 이름 변경

	UFUNCTION()
	void OnPurchaseRequested(const UShopItemDataObject* ItemDataObject);
};