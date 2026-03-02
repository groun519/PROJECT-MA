// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/InventoryItemWidget.h"
#include "Inventory/InventoryItem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Widget/InventoryItemDragDropOp.h"
#include "Widget/ItemToolTip.h"
#include "Inventory/MAItemTypes.h"
#include "Player/MAPlayerCharacter.h"     // 캐릭터 인식
#include "Player/MAPlayerController.h"
#include "Inventory/InventoryItem.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/MAFieldItem.h"
#include "Widget/MAInventoryListView.h" 

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EmptySlot();
}

void UInventoryItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	UMAInventorySlotDataObject* DataObj = Cast<UMAInventorySlotDataObject>(ListItemObject);
	if (DataObj)
	{
		UpdateInventoryItem(DataObj->InventoryItemInstance);
	}
}

bool UInventoryItemWidget::IsEmpty() const
{
	return !InventoryItem || !(InventoryItem->IsValid());
}

void UInventoryItemWidget::SetSlotNumber(int NewSlotNumber)
{
	SlotNumber = NewSlotNumber;
}

void UInventoryItemWidget::UpdateInventoryItem(const UInventoryItem* Item)
{
	UnBindCanCastAbilityDelegate();
	
	InventoryItem = Item;
	
	if (!InventoryItem || !InventoryItem->IsValid() || InventoryItem->GetStackCount() <= 0)
	{
		EmptySlot();
		return;
	}
	
	SetIcon(InventoryItem->GetIcon());
	
	if (const FBaseItemData* BaseData = InventoryItem->GetBaseData())
	{
		SetToolTipWidget(BaseData); 
	}
	
	if (InventoryItem->IsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);
		UpdateStackCount();
	}
	else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (InventoryItem->IsStackable())
	{
		StackCountText->SetVisibility(ESlateVisibility::Visible);
		UpdateStackCount();
	}
	else
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}

	ClearCooldown();
	
	if (InventoryItem->IsGrantingAnyAbility())
	{
		UpdateCanCastDisplay(InventoryItem->CanCastAbility());
		float AbilityCooldownRemaining = InventoryItem->GetAbilityCooldownTimeRemaining();
		float AbilityCooldownDuration = InventoryItem->GetAbilityCooldownDuration();

		if (AbilityCooldownRemaining > 0.f)
		{
			StartCooldown(AbilityCooldownDuration, AbilityCooldownRemaining);
		}
		
		CooldownDurationText->SetVisibility(AbilityCooldownDuration == 0.f? ESlateVisibility::Hidden : ESlateVisibility::Visible);
		CooldownDurationText->SetText(FText::AsNumber(AbilityCooldownDuration));
		BindCanCastAbilityDelegate();
	}
	else
	{
		UpdateCanCastDisplay(true);
		CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
		CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryItemWidget::EmptySlot()
{
	ClearCooldown();
	UnBindCanCastAbilityDelegate();
	InventoryItem = nullptr;
	SetIcon(EmptyTexture);
	SetToolTip(nullptr);

	StackCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryItemWidget::UpdateStackCount()
{
	if (InventoryItem)
	{
		StackCountText->SetText(FText::AsNumber(InventoryItem->GetStackCount()));
	}
}

UTexture2D* UInventoryItemWidget::GetIconTexture() const
{
	// [변경] 헬퍼 함수 사용
	if (InventoryItem)
	{
		return InventoryItem->GetIcon();
	}

	return nullptr;
}

void UInventoryItemWidget::UpdateCanCastDisplay(bool bCanCast)
{
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CanCastDynamicMaterialParamName, bCanCast ? 1.f : 0.f);
	}
}

FInventoryItemHandle UInventoryItemWidget::GetItemHandle() const
{
	if (!IsEmpty())
	{
		return InventoryItem->GetHandle();
	}

	return FInventoryItemHandle::InvalidHandle();
}

void UInventoryItemWidget::RightButtonClicked()
{
	if (!IsEmpty())
		OnRightBttonClicked.Broadcast(GetItemHandle());
}

void UInventoryItemWidget::LeftButtonClicked()
{
	if (!IsEmpty())
		OnLeftBttonClicked.Broadcast(GetItemHandle());
}

void UInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!IsEmpty() && DragDropOpClass)
	{
		UInventoryItemDragDropOp* DragDropOp = NewObject<UInventoryItemDragDropOp>(this, DragDropOpClass);
		if (DragDropOp)
		{
			DragDropOp->SetDraggedItem(this);
			OutOperation = DragDropOp;
		}
	}
}

bool UInventoryItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 1. 우리가 만든 인벤토리 드래그 오퍼레이션인지 확인
	UInventoryItemDragDropOp* DragOp = Cast<UInventoryItemDragDropOp>(InOperation);
	if (!DragOp) return false;

	// 2. 필드 아이템을 잡아서 인벤토리에 놓은 경우
	if (AMAFieldItem* FieldItem = Cast<AMAFieldItem>(DragOp->Payload))
	{
		if (AMAPlayerCharacter* PlayerChar = Cast<AMAPlayerCharacter>(GetOwningPlayerPawn()))
		{
			UInventoryComponent* InvComp = PlayerChar->GetComponentByClass<UInventoryComponent>();
			if (InvComp)
			{
				// 3. 데이터 테이블에서 아이템 기본 정보 읽어오기
				if (FieldItem->ItemDataTable && !FieldItem->ItemRowName.IsNone())
				{
					const FBaseItemData* ItemData = FieldItem->ItemDataTable->FindRow<FBaseItemData>(FieldItem->ItemRowName, TEXT("CheckItemTypeDrop"));
					
					if (ItemData)
					{
						// ==========================================================
						// [수정된 부분] 스킬이면 아예 무시하고 튕겨냅니다!
						// ==========================================================
						if (ItemData->ItemType == EMAItemType::Skill) 
						{
							// return false를 하면 드롭이 취소되고, 
							// FieldItem->Destroy()가 실행되지 않아서 바닥에 그대로 남습니다.
							return false; 
						}
						else 
						{
							// 일반 아이템(소모품, 장비 등)인 경우에만 인벤토리로 획득!
							InvComp->TryPurchaseItem(FieldItem->ItemRowName, FieldItem->ItemDataTable);
							
							// 무사히 가방에 넣었으니 바닥 액터 파괴
							FieldItem->Destroy();
							return true;
						}
					}
				}
			}
		}
	}

	// 5. 필드 아이템이 아니면 기존의 인벤토리 간 이동 로직 수행
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UInventoryItemWidget::StartCooldown(float CooldownDuration, float TimeRemaining)
{
	CooldownTimeRemaining = TimeRemaining;
	CooldownTimeDuration = CooldownDuration;
	GetWorld()->GetTimerManager().SetTimer(CooldownDurationTimerHandle, this, &UInventoryItemWidget::CooldownFinished, CooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownUpdateTimerHandle, this, &UInventoryItemWidget::UpdateCooldown, CooldownUpdateInterval, true);

	CooldownCountText->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryItemWidget::BindCanCastAbilityDelegate()
{
	if (InventoryItem)
	{
		const_cast<UInventoryItem*>(InventoryItem)->OnAbilityCanCastUpdated.AddUObject(this, &UInventoryItemWidget::UpdateCanCastDisplay);
	}
}

void UInventoryItemWidget::UnBindCanCastAbilityDelegate()
{
	if (InventoryItem)
	{
		const_cast<UInventoryItem*>(InventoryItem)->OnAbilityCanCastUpdated.RemoveAll(this);
	}
}

void UInventoryItemWidget::CooldownFinished()
{
	GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
	CooldownCountText->SetVisibility(ESlateVisibility::Hidden);
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMaterialParamName, 1.f);
	}
}

void UInventoryItemWidget::UpdateCooldown()
{
	CooldownTimeRemaining -= CooldownUpdateInterval;
	float CooldownAmt = 1.f - CooldownTimeRemaining / CooldownTimeDuration;
	CooldownDisplayFormattingOptions.MaximumFractionalDigits = CooldownTimeRemaining > 1.f ? 0 : 2;
	CooldownCountText->SetText(FText::AsNumber(CooldownTimeRemaining, &CooldownDisplayFormattingOptions));
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetScalarParameterValue(CooldownAmtDynamicMaterialParamName, CooldownAmt);
	}
}

void UInventoryItemWidget::ClearCooldown()
{
	CooldownFinished();
}


void UInventoryItemWidget::SetIcon(UTexture2D* IconTexture)
{
	if (GetItemIcon())
	{
		GetItemIcon()->GetDynamicMaterial()->SetTextureParameterValue(IconTextureDynamicMaterialParamName, IconTexture);
		return;
	}

	Super::SetIcon(IconTexture);
}
