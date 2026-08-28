// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyInviteWidget.h"
#include "Components/Button.h"
#include "Framework/MAGameInstance.h"

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
	if (UMAGameInstance* GameInstance = GetGameInstance<UMAGameInstance>())
	{
		GameInstance->ShowInviteUI();
	}
}
