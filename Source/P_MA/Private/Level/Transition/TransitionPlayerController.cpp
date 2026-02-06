// Fill out your copyright notice in the Description page of Project Settings.

#include "TransitionPlayerController.h"
#include "Framework/MAGameInstance.h"

void ATransitionPlayerController::BeginPlay()
{
	Super::BeginPlay();


	if (IsLocalController())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ATransitionPlayerController::DelayedStartLoadingScreen);
	}

	GetWorldTimerManager().SetTimer(
		LoadingStatusTimerHandle,
		this,
		&ATransitionPlayerController::TickLoadingStatus,
		0.2f,
		true
	);
}

void ATransitionPlayerController::DelayedStartLoadingScreen()
{
	if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
	{
		GI->StartLoadingScreen();
	}
}

void ATransitionPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LoadingStatusTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ATransitionPlayerController::TickLoadingStatus()
{
	if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
	{
		GI->UpdateLoadingStatus();
	}
}
