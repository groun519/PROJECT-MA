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
		// 사용예시. 매크로로 함수 쉽게 넘김.
		InteractComp->CALL_SETUP_INTERACT(HandleInteract);
	}
}

void ACore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACore::HandleInteract(AMAPlayerCharacter* Interactor)
{
	UE_LOG(LogTemp, Display, TEXT("Core Interacted!"));
	// 여기에 추가해주면 됩니다 용범BROTHER 
}