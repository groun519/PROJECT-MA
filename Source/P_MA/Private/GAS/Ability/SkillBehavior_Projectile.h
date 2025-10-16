// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "GAS/MAGameplayAbility.h"
#include "SkillBehavior_Projectile.generated.h"

class MABaseProjectile;
class AMACharacter;

/**
 * 
 */
UCLASS()
class USkillBehavior_Projectile : public UMASkillBehavior
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
	TSubclassOf<class AMABaseProjectile> ProjectileClass;
	

	UPROPERTY(EditDefaultsOnly)
	FName MuzzleSocketName;

	

};
