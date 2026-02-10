// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LobbyAvatarState.h"
#include "LobbyAvatarAnimInstance.generated.h"

UCLASS()
class P_MA_API ULobbyAvatarAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetLobbyState(ELobbyAvatarState NewState);

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe), Category = "Lobby")
	bool IsState(ELobbyAvatarState TargetState) const;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	ELobbyAvatarState LobbyState = ELobbyAvatarState::Wait;
};
