// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Abilities/GameplayAbility.h"
#include "MAPlayerState.generated.h"

UCLASS()
class P_MA_API AMAPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill);
	TSubclassOf<UGameplayAbility> GetDefaultSkill() const { return DefaultSkill; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_DefaultSkill)
	TSubclassOf<UGameplayAbility> DefaultSkill;

	UFUNCTION()
	void OnRep_DefaultSkill();
};
