// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyAvatarAnimInstance.h"

void ULobbyAvatarAnimInstance::SetLobbyState(ELobbyAvatarState NewState)
{
	LobbyState = NewState;
}

bool ULobbyAvatarAnimInstance::IsState(ELobbyAvatarState TargetState) const
{
	return LobbyState == TargetState;
}
