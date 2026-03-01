// Fill out your copyright notice in the Description page of Project Settings.

#include "MAPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Framework/MAGameInstance.h"

// NOTE:
// Seamless travel 과정에서 새 PlayerState 인스턴스로 교체될 때
// 로드아웃/슬롯 정보가 기본값으로 돌아가지 않게 수동 복사한다.
void AMAPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	AMAPlayerState* NewPS = Cast<AMAPlayerState>(PlayerState);
	if (!NewPS) return;

	NewPS->DefaultSkill = DefaultSkill;
	NewPS->LoadoutColor = LoadoutColor;
	NewPS->LoadoutWeaponId = LoadoutWeaponId;
	NewPS->bHasFinishedLoading = bHasFinishedLoading;
	NewPS->LobbySlotIndex = LobbySlotIndex;
}

void AMAPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	const AMAPlayerState* OldPS = Cast<AMAPlayerState>(PlayerState);
	if (!OldPS) return;

	DefaultSkill = OldPS->DefaultSkill;
	LoadoutColor = OldPS->LoadoutColor;
	LoadoutWeaponId = OldPS->LoadoutWeaponId;
	bHasFinishedLoading = OldPS->bHasFinishedLoading;
	LobbySlotIndex = OldPS->LobbySlotIndex;
}

void AMAPlayerState::SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill)
{
	DefaultSkill = NewSkill;
}

void AMAPlayerState::SetLoadoutColor(const FMaterialParamDataPair& NewColor)
{
	LoadoutColor = NewColor;
	OnLoadoutColorChanged.Broadcast(LoadoutColor);
}

void AMAPlayerState::SetLoadoutWeaponId(FName NewWeaponId)
{
	LoadoutWeaponId = NewWeaponId;
	OnLoadoutWeaponChanged.Broadcast(LoadoutWeaponId);
}

void AMAPlayerState::SetLoadingComplete(bool bComplete)
{
	bHasFinishedLoading = bComplete;
}

void AMAPlayerState::SetLobbySlotIndex(int32 Index)
{
	LobbySlotIndex = Index;
}


void AMAPlayerState::OnRep_DefaultSkill()
{
}

void AMAPlayerState::OnRep_LoadoutColor()
{
	OnLoadoutColorChanged.Broadcast(LoadoutColor);
}

void AMAPlayerState::OnRep_LoadoutWeaponId()
{
	OnLoadoutWeaponChanged.Broadcast(LoadoutWeaponId);
}

void AMAPlayerState::OnRep_LoadingComplete()
{
	if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
	{
		GI->UpdateLoadingStatus();
	}
}

void AMAPlayerState::OnRep_LobbySlotIndex()
{
}


void AMAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMAPlayerState, DefaultSkill);
	DOREPLIFETIME(AMAPlayerState, LoadoutColor);
	DOREPLIFETIME(AMAPlayerState, LoadoutWeaponId);
	DOREPLIFETIME(AMAPlayerState, bHasFinishedLoading);
	DOREPLIFETIME(AMAPlayerState, LobbySlotIndex);
}
