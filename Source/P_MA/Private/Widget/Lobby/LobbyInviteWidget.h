// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyInviteWidget.generated.h"

class UButton;

UCLASS()
class P_MA_API ULobbyInviteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InviteButton;

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleInviteClicked();
};
