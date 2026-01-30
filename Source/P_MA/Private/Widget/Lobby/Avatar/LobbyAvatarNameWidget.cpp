// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyAvatarNameWidget.h"
#include "Components/TextBlock.h"

void ULobbyAvatarNameWidget::SetNameText(const FString& NewText)
{
	if (!NameText) return;
	NameText->SetText(FText::FromString(NewText));
}
