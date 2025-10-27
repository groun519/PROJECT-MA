// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "GAS/MAGameplayAbility.h"
#include "SkillBehavior_SpawnForwardActor.generated.h"


class AMACharacter;

/**
 * 전방 액터 스폰
 * 설정한 투사체를 캐릭터 앞에서 스폰
 */
UCLASS()
class USkillBehavior_SpawnForwardActor : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	
protected:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> ProjectileEventTask;

	FGameplayTag ProjectileTag = FGameplayTag::RequestGameplayTag("Event.Montage.SpawnProjectile");
	
	UFUNCTION()
	void OnProjectileEventReceived(FGameplayEventData EventData);

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMAProjectile_OverlapAOE> ProjectileClass;
	

	UPROPERTY(EditDefaultsOnly)
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly)
	float AbilitySize = 300.f;
};
