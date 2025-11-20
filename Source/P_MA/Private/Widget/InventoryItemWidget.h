// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/ItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "InventoryItemWidget.generated.h"

class UInventoryItem;
class UInventoryItemWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemDropped, UInventoryItemWidget* /*DestionationWidget*/, UInventoryItemWidget* /*SourceWidget*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnButtonClick, const FInventoryItemHandle& /*ItemHandle*/);
/**
 * 
 */
UCLASS()
class UInventoryItemWidget : public UItemWidget
{
	GENERATED_BODY()
public:
	FOnInventoryItemDropped OnInventoryItemDropped;
	FOnButtonClick OnLeftBttonClicked;
	FOnButtonClick OnRightBttonClicked;
	virtual void NativeConstruct() override;
	bool IsEmpty() const;
	void SetSlotNumber(int NewSlotNumber);
	void UpdateInventoryItem(const UInventoryItem* Item);
	void EmptySlot();
	FORCEINLINE int GetSlotNumber() const { return SlotNumber; }
	void UpdateStackCount();

	UTexture2D* GetIconTexture() const;
	FORCEINLINE const UInventoryItem* GetInventoryItem() const { return InventoryItem; }
	FInventoryItemHandle GetItemHandle() const;

private:

	void UpdateCanCastDisplay(bool bCanCast);
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	UTexture2D* EmptyTexture;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* StackCountText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownCountText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownDurationText;
	
	UPROPERTY()
	const UInventoryItem* InventoryItem;

	int SlotNumber;

	virtual void RightButtonClicked() override;
	virtual void LeftButtonClicked() override;
	/******************************************/
	/*           Drag Drop                    */
	/******************************************/
private:
	virtual void NativeOnDragDetected( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation ) override;
	virtual bool NativeOnDrop( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation ) override;

	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	TSubclassOf<class UInventoryItemDragDropOp> DragDropOpClass;

	/******************************************/
	/*            GAS                         */
	/******************************************/

public:
	void StartCooldown(float CooldownDuration, float TimeRemaining);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownUpdateInterval = 0.1f;

	void BindCanCastAbilityDelegate();
	void UnBindCanCastAbilityDelegate();

	void CooldownFinished();
	void UpdateCooldown();
	void ClearCooldown();

	FTimerHandle CooldownDurationTimerHandle;
	FTimerHandle CooldownUpdateTimerHandle;

	float CooldownTimeRemaining = 0.f;
	float CooldownTimeDuration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FName CooldownAmtDynamicMaterialParamName = "Percent";

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FName IconTextureDynamicMaterialParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	FName CanCastDynamicMaterialParamName = "CanCast";

	virtual void SetIcon(UTexture2D* IconTexture) override;
	FNumberFormattingOptions CooldownDisplayFormattingOptions;
};