// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Modules/MASkillModuleData.h"
#include "MAGameplayAbility_Skill.generated.h"

class UMASkillModule;
/**
 * 
 */
UCLASS()
class UMAGameplayAbility_Skill : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UMAGameplayAbility_Skill();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	const FSkillData& GetSkillData() const {return CachedSkillData;}
	const FModuleBehaviorData& GetBehaviorData() const {return CachedBehaviorData;}
	const FModuleElementalData& GetElementalData() const {return CachedElementalData;}
	const FModuleUtilityData& GetUtilityData() const {return CachedUtilityData;}
	
	UFUNCTION()
	float GetTotalAnimSpeed() const;
	
	void ApplyDamageToHitResults(const TArray<FHitResult>& HitResults, float DamageMultiplier = 1.f);
	void ExecuteSkillAction(FGameplayEventData& Payload, float BehaviorMultiplier = 1.f);
	
protected:
	TSubclassOf<UGameplayEffect> GetBaseDamageEffect() const;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMASkillModule>> ActiveModules;
	UPROPERTY(Transient)
	FSkillData CachedSkillData;
	UPROPERTY(Transient)
	FModuleBehaviorData CachedBehaviorData;
	UPROPERTY(Transient)
	FModuleElementalData CachedElementalData;
	UPROPERTY(Transient)
	FModuleUtilityData CachedUtilityData;

	UPROPERTY(EditDefaultsOnly, Category="Config")
	FName SkillID;

	void PerformMeleeAttack(FGameplayEventData& Payload, float FinalMultiplier);
	void SpawnProjectile(FGameplayEventData& Payload, float DamageMultiplier);
	void SpawnTargetingProjectile(FGameplayEventData& Payload, float DamageMultiplier);
	void SpawnProjectileActor(TSubclassOf<AActor> Class, FVector Loc, FRotator Rot, float DamageMultiplier);
	bool LoadSkillData();

	FGameplayEffectSpecHandle MakeSkillDamageSpec(float BehaviorMultiplier);
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitClearEventTask;
	UFUNCTION()
	void TargetClear(FGameplayEventData Payload);
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitVFXEventTask;
	UFUNCTION()
	void HandleVFXSpawnEvent(FGameplayEventData Payload);

	FGameplayTag VFXRootTag;
	FGameplayTag IgnoreClearTag;
	
public:
	void Montage_SetPlayRate(UAnimMontage* AnimMontage, float PlayRate);
	void Montage_SetSection(FName SectionName);

	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
};
