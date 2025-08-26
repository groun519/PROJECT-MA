// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MAPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Player/MAPlayerCharacter.h"
#include "MAGameplayWidget.h"

void AMAPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		MAPlayerCharacter->ServerSideInit();
	}
}

void AMAPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		MAPlayerCharacter->ClientSideInit();
		//SpawnHUDWidget();
		SpawnGameplayWidget();
	}
}

//void AMAPlayerController::SpawnHUDWidget()
//{
//	if (!IsLocalPlayerController()) return;
//
//	HUDWidget = CreateWidget<UMAHUD>(this, HUDWidgetClass);
//	if (HUDWidget)
//	{
//		HUDWidget->AddToViewport();
//	}
//}
void AMAPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController())
		return;

	GameplayWidget = CreateWidget<UMAGameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
	}
}

