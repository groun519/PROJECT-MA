// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MAPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Player/MAPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Widget/MAHUD.h"

void AMAPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		MAPlayerCharacter->ServerSideInit();
		MAPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void AMAPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		MAPlayerCharacter->ClientSideInit();
		SpawnHUDWidget();
	}
}

void AMAPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId AMAPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void AMAPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAPlayerController, TeamID);
}

void AMAPlayerController::SpawnHUDWidget()
{
	if (!IsLocalPlayerController()) return;

	HUDWidget = CreateWidget<UMAHUD>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}
}

