// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyAvatarReadyWidget.h"
#include "Components/TextBlock.h"

void ULobbyAvatarReadyWidget::SetLobbyState(ELobbyAvatarState State)
{
	if (!ReadyText)
	{
		return;
	}

	if (State == ELobbyAvatarState::Ready)
	{
		ReadyText->SetText(FText::FromString(TEXT("Ready!")));
		ReadyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.1f, 0.9f, 0.1f, 1.0f)));
	}
	else if (State == ELobbyAvatarState::Loadout)
	{
		ReadyText->SetText(FText::FromString(TEXT("Loadout")));
		ReadyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.55f, 0.1f, 1.0f)));
	}
	else
	{
		ReadyText->SetText(FText::FromString(TEXT("Wait..")));
		ReadyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.1f, 0.1f, 1.0f)));
	}
}
