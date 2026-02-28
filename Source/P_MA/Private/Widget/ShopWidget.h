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

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopClosedDelegate);
UCLASS()
class UShopWidget : public UMovableWindowWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitShop(const TArray<UDataTable*>& InDataTables);

	UPROPERTY(BlueprintAssignable, Category = "Shop")
	FOnShopClosedDelegate OnShopClosed;
private:
	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UFUNCTION()
	void OnCloseClicked();
	
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* CategoryContainer; 
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShopCategoryWidget> CategoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"))
	TArray<UDataTable*> ShopDataTables;

	UPROPERTY()
	UInventoryComponent* OwnerInventoryComponent;

	void LoadShopCategories(); 

	UFUNCTION()
	void OnPurchaseRequested(const UShopItemDataObject* ItemDataObject);

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* ShopAnim;
};