// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Widget/ItemWidget.h"
#include "Inventory/MAItemTypes.h" // [필수] 구조체 포함
#include "ShopItemWidget.generated.h"

class UShopItemWidget;
class UDataTable;

/**
 * [변경] 리스트 뷰에 들어갈 데이터 객체
 * 이제 아이템의 "주소" (어느 테이블의, 어느 행인지)를 저장합니다.
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

	// UI 표시용 캐시 데이터
	const FBaseItemData* CachedItemData = nullptr;
};

// 구매 요청 델리게이트 (데이터 객체를 통째로 넘김)
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