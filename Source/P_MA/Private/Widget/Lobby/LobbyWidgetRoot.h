// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidgetRoot.generated.h"

class ULobbyReadyStartWidget;
class ULoadoutWidget;
class UButton;
class UTextBlock;

UCLASS()
class P_MA_API ULobbyWidgetRoot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULobbyReadyStartWidget> LobbyReadyStartWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoadoutButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoadoutButtonText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<ULoadoutWidget> LoadoutWidget;
};
