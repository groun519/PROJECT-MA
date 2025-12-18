// Fill out your copyright notice in the Description page of Project Settings.

#include "Core.h"
#include "Convenience/InteractComponent.h"

ACore::ACore()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractComp = CreateDefaultSubobject<UInteractComponent>("InteractRangeSphere");
	InteractComp->SetupAttachment(GetRootComponent());
}

void ACore::BeginPlay()
{
	Super::BeginPlay();
	if (InteractComp)
	{
		InteractComp->OnInteractRequested.AddDynamic(this, &ACore::HandleInteract);
	}
}

void ACore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACore::HandleInteract(AMAPlayerCharacter* Interactor)
{
	UE_LOG(LogTemp, Display, TEXT("Interact"));
}