// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Level/Lobby/LobbyAvatarState.h"
#include "LobbyAvatarReadyWidget.generated.h"

class UTextBlock;

UCLASS()
class P_MA_API ULobbyAvatarReadyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetLobbyState(ELobbyAvatarState State);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyText;
};
