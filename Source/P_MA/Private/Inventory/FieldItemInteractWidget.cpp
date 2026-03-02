// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/FieldItemInteractWidget.h"
#include "Inventory/MAFieldItem.h"
#include "Widget/InventoryItemDragDropOp.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h" // [필수] 이미지 위젯용
#include "Components/Border.h" // [옵션] 아이콘 테두리용 (없으면 메쉬 배경이 보일 수 있음)
#include "Components/WidgetComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

void UFieldItemInteractWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true); // [핵심 1] 위젯이 마우스 드래그를 잡을 수 있도록 허용
}

FReply UFieldItemInteractWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 클릭이 제대로 감지되는지 화면 왼쪽 위에 초록색 글로 띄워봅니다.
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("1. 클릭 감지됨!"));
		
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFieldItemInteractWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (!OwnerFieldItem) return;

	// ==========================================================
	// [수정된 부분] 여기에 진짜 거리 체크 로직을 다시 넣습니다!
	// ==========================================================
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetPawn())
	{
		// 500 유닛(cm)보다 멀리 떨어져 있으면 드래그를 취소합니다.
		if (OwnerFieldItem->GetDistanceTo(PC->GetPawn()) > 500.f)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("실패: 아이템이 너무 멉니다!"));
			return; // 여기서 함수를 끝내버려서 드래그를 막습니다.
		}
	}
	// ==========================================================

	UInventoryItemDragDropOp* DragOp = Cast<UInventoryItemDragDropOp>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UInventoryItemDragDropOp::StaticClass())
	);

    if (DragOp)
    {
        DragOp->Payload = OwnerFieldItem;

        // ====================================================================
        // [핵심 4] 데이터 테이블에서 진짜 아이콘 가져오기
        // ====================================================================
        if (OwnerFieldItem->ItemDataTable && !OwnerFieldItem->ItemRowName.IsNone())
        {
            // "FieldItem_Drag" 라는 ContextString은 에러 추적용 이름입니다.
            const FBaseItemData* ItemData = OwnerFieldItem->ItemDataTable->FindRow<FBaseItemData>(
                OwnerFieldItem->ItemRowName, TEXT("FieldItem_Drag")
            );

            // 아이템 데이터가 있고, 아이콘(Soft Object Pointer)이 비어있지 않다면
            if (ItemData && !ItemData->Icon.IsNull())
            {
                // 1. 임시 배경(Border) 생성 - 3D 화면과 구분감을 주기 위함
                UBorder* DragVisualRoot = NewObject<UBorder>(this);
                DragVisualRoot->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.5f)); // 반투명 검정 배경
                DragVisualRoot->SetPadding(FMargin(2.f));

                // 2. 이미지 컴포넌트 생성 및 텍스쳐 로드
                UImage* DragVisualIcon = NewObject<UImage>(DragVisualRoot);
                
                // LoadSynchronous()로 Soft Pointer의 이미지를 메모리에 즉시 올립니다.
                UTexture2D* LoadedIcon = ItemData->Icon.LoadSynchronous(); 
                if (LoadedIcon)
                {
                    DragVisualIcon->SetBrushFromTexture(LoadedIcon);
                    DragVisualIcon->SetBrushSize(FVector2D(64.f, 64.f)); // 인벤토리 슬롯 크기에 맞게 조절하세요 (예: 64x64)
                }

                // 3. 배경 안에 이미지 집어넣기
                DragVisualRoot->SetContent(DragVisualIcon);

                // 4. 드래그 오퍼레이션의 비주얼로 등록! ("잡았다" 텍스트 안녕~)
                DragOp->DefaultDragVisual = DragVisualRoot;
            }
        }

        OutOperation = DragOp;
    }
}
void UFieldItemInteractWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    
	if (OwnerFieldItem)
	{
		// 1. 처음 마우스를 올렸을 때 딱 한 번! 3D 액터에서 툴팁을 떼어옵니다.
		if (!CachedToolTipWidget)
		{
			if (UWidgetComponent* ToolTipComp = OwnerFieldItem->GetToolTipWidgetComp())
			{
				CachedToolTipWidget = ToolTipComp->GetUserWidgetObject();
                
				// 떼어냈으니 3D 월드 컴포넌트에서는 연결을 끊어버립니다. (충돌 방지)
				ToolTipComp->SetWidget(nullptr); 
			}
		}

		// 2. 떼어온 툴팁을 '마우스 커서 전용 툴팁'으로 부착합니다!
		if (CachedToolTipWidget)
		{
			SetToolTip(CachedToolTipWidget);
		}
	}
}

void UFieldItemInteractWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
    
	// 마우스가 나가면 툴팁을 끕니다. (위젯 데이터는 CachedToolTipWidget에 안전하게 남아있음)
	SetToolTip(nullptr);
}
