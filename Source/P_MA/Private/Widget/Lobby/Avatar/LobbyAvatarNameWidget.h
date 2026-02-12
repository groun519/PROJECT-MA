// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyAvatarNameWidget.generated.h"

class UTextBlock;

UCLASS()
class P_MA_API ULobbyAvatarNameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetNameText(const FString& NewText);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;
};
