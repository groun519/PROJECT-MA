// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_LobbyWeaponOnlyOwnerSee.h"
#include "LobbyAvatarSlot.h"

void UAnimNotify_LobbyWeaponOnlyOwnerSee::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp)
	{
		return;
	}

	if (ALobbyAvatarSlot* Slot = Cast<ALobbyAvatarSlot>(MeshComp->GetOwner()))
	{
		Slot->SetWeaponOnlyOwnerSee(bOnlyOwnerSee);
	}
}
