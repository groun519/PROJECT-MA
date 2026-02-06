// Fill out your copyright notice in the Description page of Project Settings.

#include "TransitionGameMode.h"
#include "TransitionPlayerController.h"

ATransitionGameMode::ATransitionGameMode()
{
	PlayerControllerClass = ATransitionPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
}

void ATransitionGameMode::BeginPlay()
{
	Super::BeginPlay();
}
