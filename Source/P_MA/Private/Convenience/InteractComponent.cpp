#include "InteractComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "P_MA/P_MA.h"

UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	InitSphereRadius(150.0f);
	
	InteractKeyWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractKeyWidgetComp"));
	// 사실상 런타임에서는 의미 없지만 유지.
	if (InteractKeyWidgetComp)
	{
		InteractKeyWidgetComp->SetupAttachment(this);
		InteractKeyWidgetComp->SetVisibility(false);
		InteractKeyWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); 
	}
}

void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (InteractKeyWidgetComp)
	{
		InteractKeyWidgetComp->SetRelativeLocation(FVector::ZeroVector);
		InteractKeyWidgetComp->SetVisibility(false);
	}
	
	OnComponentBeginOverlap.AddDynamic(this, &UInteractComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractComponent::HandleEndOverlap);
}

void UInteractComponent::OnRegister()
{
	if (InteractKeyWidgetComp && InteractKeyWidgetComp->GetAttachParent() != this)
	{
		InteractKeyWidgetComp->SetupAttachment(this);
	}

	Super::OnRegister();
}

void UInteractComponent::RequestInteract(AMAPlayerCharacter* Interactor)
{
	if (InteractionHandler) InteractionHandler(Interactor);
}

void UInteractComponent::SetActive(bool bNewActive)
{
	if (bActive == bNewActive) return;
	bActive = bNewActive;
	if (InteractKeyWidgetComp) InteractKeyWidgetComp->SetVisibility(bActive);
}

void UInteractComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor))
	{
		Player->SetCurrentInteractComp(this);
		SetActive(true);
	}
}

void UInteractComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor))
	{
		Player->ClearCurrentInteractComp(this);
		SetActive(false);
	}
}
