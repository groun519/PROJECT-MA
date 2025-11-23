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

// 구매 요청을 부모(ShopWidget)에게 전달하기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategoryPurchaseRequested, const UShopItemDataObject*, ItemDataObj);

UCLASS()
class UShopCategoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 외부에서 테이블과 제목을 넣어 초기화하는 함수
	void InitCategory(UDataTable* InDataTable);

	// 구매 신호 전달용
	FOnCategoryPurchaseRequested OnCategoryPurchaseRequested;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CategoryTitleText;

	UPROPERTY(meta = (BindWidget))
	UTileView* CategoryItemList;

	// [!!!수정 1!!!] UFUNCTION() 삭제 (네이티브 델리게이트는 이게 필요 없음)
	// [!!!수정 2!!!] UUserWidget* -> UUserWidget& (참조로 변경)
	void HandleItemGenerated(UUserWidget& NewWidget);

	UFUNCTION()
	void RelayPurchaseRequest(const UShopItemDataObject* ItemDataObj);
};