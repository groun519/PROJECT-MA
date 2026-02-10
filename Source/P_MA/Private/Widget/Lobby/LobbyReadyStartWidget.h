// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "LobbyReadyStartWidget.generated.h"

class UTextBlock;

UCLASS()
class P_MA_API ULobbyReadyStartWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyStartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyStartText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyStatusText;

	void UpdateStatus(bool bIsHost, bool bIsReady, int32 ReadyCount, int32 TotalCount);
};
