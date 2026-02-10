// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_LobbyWeaponOnlyOwnerSee.generated.h"

UCLASS()
class P_MA_API UAnimNotify_LobbyWeaponOnlyOwnerSee : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Lobby")
	bool bOnlyOwnerSee = true;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
