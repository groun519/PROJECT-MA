// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/MAPlayerCharacter.h"


UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetGenerateOverlapEvents(true);
	UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UPrimitiveComponent::SetCollisionResponseToAllChannels(ECR_Ignore);
	UPrimitiveComponent::SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}


void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UInteractComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractComponent::HandleEndOverlap);
}

void UInteractComponent::OnRegister()
{
	Super::OnRegister();

	if (IsTemplate()) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	if (!InteractKeyWidgetComp)
	{
		InteractKeyWidgetComp = NewObject<UWidgetComponent>(Owner, TEXT("InteractKeyWidgetComp"));
		if (!InteractKeyWidgetComp) return;

		Owner->AddInstanceComponent(InteractKeyWidgetComp);
		InteractKeyWidgetComp->RegisterComponent();
		InteractKeyWidgetComp->SetVisibility(false, true);
	}

	InteractKeyWidgetComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	InteractKeyWidgetComp->SetRelativeLocation(FVector::ZeroVector);
	InteractKeyWidgetComp->SetRelativeRotation(FRotator::ZeroRotator);

	InteractKeyWidgetComp->SetVisibility(false);
}

void UInteractComponent::RequestInteract(AMAPlayerCharacter* Interactor)
{
	OnInteractRequested.Broadcast(Interactor);
}

void UInteractComponent::SetActive(bool bNewActive, AMAPlayerCharacter* Interactor)
{
	if (bActive == bNewActive)
		return;

	bActive = bNewActive;

	if (bActive)
	{
		ShowInteractKeyUI(Interactor);
	}
	else
	{
		HideInteractKeyUI();
	}
}

void UInteractComponent::ShowInteractKeyUI(AMAPlayerCharacter* Interactor)
{
	if (!InteractKeyWidgetComp) return;
	InteractKeyWidgetComp->SetVisibility(true, true);
}

void UInteractComponent::HideInteractKeyUI()
{
	if (!InteractKeyWidgetComp) return;
	InteractKeyWidgetComp->SetVisibility(false, true);
}

void UInteractComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor);
	if (!Player) return;

	Player->SetCurrentInteractComp(this);
}

void UInteractComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor);
	if (!Player) return;

	Player->ClearCurrentInteractComp(this);
}
