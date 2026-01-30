// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidgetRoot.generated.h"

class ULobbyReadyStartWidget;

UCLASS()
class P_MA_API ULobbyWidgetRoot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULobbyReadyStartWidget> LobbyReadyStartWidget;
};
