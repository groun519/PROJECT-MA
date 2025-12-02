// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "GAS/MAGameplayAbility.h"
#include "SkillBehavior_SpawnActorFwd.generated.h"


class AMACharacter;

/**
 * 전방 액터 스폰
 * 설정한 투사체를 캐릭터 앞에서 스폰
 */
UCLASS()
class USkillBehavior_SpawnActorFwd : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual void InitFromConfig(const FInstancedStruct& ConfigPayload) override;

	
protected:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> ProjectileEventTask;

	FGameplayTag ProjectileTag = FGameplayTag::RequestGameplayTag("Event.Montage.SpawnProjectile");
	
	UFUNCTION()
	void OnProjectileEventReceived(FGameplayEventData EventData);

private:
	UPROPERTY()
	TSubclassOf<AMAProjectile_OverlapAOE> DefaultProjectile;
	UPROPERTY()
	TMap<FName, TSubclassOf<AMAProjectile_OverlapAOE>> ElementalProjectiles;
	UPROPERTY()
	TSubclassOf<AMAProjectile_OverlapAOE> ProjectileToSpawn;
	
	float ProjectileSpeed;
	float ProjectileMaxDist;
	float ExplodeRadius;
	float SpawnDelay;
	int32 ProjectileCount;
	FName MuzzleSocketName;
};
