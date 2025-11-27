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
	virtual void InitFromData(const FSkillDefinitionDT& Data) override;
	
protected:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> ProjectileEventTask;

	FGameplayTag ProjectileTag = FGameplayTag::RequestGameplayTag("Event.Montage.SpawnProjectile");
	
	UFUNCTION()
	void OnProjectileEventReceived(FGameplayEventData EventData);

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMAProjectile_OverlapAOE> DefaultProjectile;
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<class AMAProjectile_OverlapAOE>> ProjectileClasses;
	
	TMap<FName, TSubclassOf<AMAProjectile_OverlapAOE>> ElementalProjectiles;
	
	UPROPERTY(EditDefaultsOnly)
	float ProjectileSpeed = 700.f;
	UPROPERTY(EditDefaultsOnly)
	float ProjectileMaxDist = 3000.f;
	UPROPERTY(EditDefaultsOnly)
	float ExplodeRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly)
	FName MuzzleSocketName;

	int32 ProjectileCount;
	float SpawnDelay;
};
