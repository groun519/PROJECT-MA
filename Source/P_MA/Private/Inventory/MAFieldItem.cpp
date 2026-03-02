// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/MAFieldItem.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Widget/ItemToolTip.h" 
#include "Inventory/FieldItemInteractWidget.h"

AMAFieldItem::AMAFieldItem()
{
	PrimaryActorTick.bCanEverTick = false;

	// 메쉬 설정 및 마우스 반응 활성화
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 툴팁 위젯 컴포넌트 설정
	ToolTipWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ToolTipWidget"));
	ToolTipWidgetComp->SetupAttachment(RootComponent);
	ToolTipWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	ToolTipWidgetComp->SetDrawAtDesiredSize(true);
	ToolTipWidgetComp->SetVisibility(false); // 평소엔 숨김

	// 마우스 오버 이벤트 연결
	OnBeginCursorOver.AddDynamic(this, &AMAFieldItem::OnBeginMouseOver);
	OnEndCursorOver.AddDynamic(this, &AMAFieldItem::OnEndMouseOver);

	InteractWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidgetComp->SetupAttachment(RootComponent);
	InteractWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); // 화면 고정
	InteractWidgetComp->SetDrawSize(FVector2D(150.f, 150.f));

	bReplicates = true;
}

void AMAFieldItem::BeginPlay()
{
	Super::BeginPlay();

	// 위젯에 아이템 데이터 세팅
	if (ItemDataTable && !ItemRowName.IsNone())
	{
		UItemToolTip* ToolTipWidget = Cast<UItemToolTip>(ToolTipWidgetComp->GetUserWidgetObject());
		if (ToolTipWidget)
		{
			const FBaseItemData* Data = ItemDataTable->FindRow<FBaseItemData>(ItemRowName, TEXT("FieldItem_Init"));
			ToolTipWidget->SetItemData(Data);
		}
	}

	if (UFieldItemInteractWidget* InteractWidget = Cast<UFieldItemInteractWidget>(InteractWidgetComp->GetUserWidgetObject()))
	{
		InteractWidget->OwnerFieldItem = this;
	}
}

void AMAFieldItem::OnBeginMouseOver(AActor* TouchedActor)
{
	if (ToolTipWidgetComp) ToolTipWidgetComp->SetVisibility(true);
}

void AMAFieldItem::OnEndMouseOver(AActor* TouchedActor)
{
	if (ToolTipWidgetComp) ToolTipWidgetComp->SetVisibility(false);
}


void AMAFieldItem::SetToolTipVisible(bool bVisible)
{
	if (ToolTipWidgetComp)
	{
		ToolTipWidgetComp->SetVisibility(bVisible);
	}
}

void AMAFieldItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// ItemRowName을 서버에서 클라이언트로 동기화
	DOREPLIFETIME(AMAFieldItem, ItemRowName);
}