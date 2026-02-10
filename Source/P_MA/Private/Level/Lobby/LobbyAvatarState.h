// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "LobbyAvatarState.generated.h"

UENUM(BlueprintType)
enum class ELobbyAvatarState : uint8
{
	Wait,
	Ready,
	Loadout
};
