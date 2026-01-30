// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

UCLASS()
class P_MA_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bNewReady);

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void ShowLobbyUI();

protected:
	UPROPERTY(EditAnywhere, Category = "Lobby")
	TSubclassOf<class ULobbyWidgetRoot> LobbyRootWidgetClass;

	UPROPERTY()
	TObjectPtr<ULobbyWidgetRoot> LobbyRootWidgetInstance;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	FName LobbyCameraTag = TEXT("LobbyCamera");
};
