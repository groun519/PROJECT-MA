// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyPlayerController.h"
#include "LobbyGameState.h"
#include "GameFramework/PlayerState.h"
#include "Widget/LobbyWidgetRoot.h"
#include "Kismet/GameplayStatics.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ShowLobbyUI();

	if (LobbyRootWidgetClass)
	{
		LobbyRootWidgetInstance = CreateWidget<ULobbyWidgetRoot>(this, LobbyRootWidgetClass);
		if (LobbyRootWidgetInstance)
		{
			LobbyRootWidgetInstance->AddToViewport();
		}
	}

	TArray<AActor*> TaggedActors;
	UGameplayStatics::GetAllActorsWithTag(this, LobbyCameraTag, TaggedActors);
	if (TaggedActors.Num() > 0 && TaggedActors[0])
	{
		SetViewTargetWithBlend(TaggedActors[0], 0.0f);
	}
}

void ALobbyPlayerController::SetReady(bool bNewReady)
{
	ServerSetReady(bNewReady);
}

void ALobbyPlayerController::ServerSetReady_Implementation(bool bNewReady)
{
	if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
	{
		LGS->SetPlayerReady(GetPlayerState<APlayerState>(), bNewReady);
	}
}
