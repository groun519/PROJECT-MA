// Fill out your copyright notice in the Description page of Project Settings.

#include "MAPlayerState.h"
#include "Net/UnrealNetwork.h"

void AMAPlayerState::SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill)
{
	DefaultSkill = NewSkill;
}

void AMAPlayerState::SetLoadoutColor(const FMaterialParamDataPair& NewColor)
{
	LoadoutColor = NewColor;
	UE_LOG(LogTemp, Warning, TEXT("Loadout: PS SetLoadoutColor PS=%s Role=%d Body=%s Eye=%s"),
		*GetNameSafe(this),
		static_cast<int32>(GetLocalRole()),
		*LoadoutColor.BodyData.Color.ToString(),
		*LoadoutColor.EyeData.Color.ToString());
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
	UE_LOG(LogTemp, Warning, TEXT("Loadout: PS OnRep_LoadoutColor PS=%s Role=%d Body=%s Eye=%s"),
		*GetNameSafe(this),
		static_cast<int32>(GetLocalRole()),
		*LoadoutColor.BodyData.Color.ToString(),
		*LoadoutColor.EyeData.Color.ToString());
	OnLoadoutColorChanged.Broadcast(LoadoutColor);
}

void AMAPlayerState::OnRep_LoadoutWeaponId()
{
	OnLoadoutWeaponChanged.Broadcast(LoadoutWeaponId);
}

void AMAPlayerState::OnRep_LoadingComplete()
{
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
