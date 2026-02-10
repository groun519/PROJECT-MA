// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyInviteWidget.h"
#include "Components/Button.h"
#include "Level/Lobby/LobbyPlayerController.h"

void ULobbyInviteWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (InviteButton)
	{
		InviteButton->OnClicked.AddDynamic(this, &ULobbyInviteWidget::HandleInviteClicked);
	}
}

void ULobbyInviteWidget::HandleInviteClicked()
{
	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		PC->ShowInviteUI();
	}
}
