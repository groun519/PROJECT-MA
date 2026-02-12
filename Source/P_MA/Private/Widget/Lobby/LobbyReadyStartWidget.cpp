// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyReadyStartWidget.h"
#include "Components/TextBlock.h"
void ULobbyReadyStartWidget::UpdateStatus(bool bIsHost, bool bIsReady, int32 ReadyCount, int32 TotalCount)
{
	if (ReadyStartText)
	{
		const FString ButtonLabel = bIsHost ? TEXT("Play") : (bIsReady ? TEXT("Cancel") : TEXT("Ready"));
		ReadyStartText->SetText(FText::FromString(ButtonLabel));
	}

	if (ReadyStatusText)
	{
		const FString Status = FString::Printf(TEXT("%d / %d"), ReadyCount, TotalCount);
		ReadyStatusText->SetText(FText::FromString(Status));
	}
}
