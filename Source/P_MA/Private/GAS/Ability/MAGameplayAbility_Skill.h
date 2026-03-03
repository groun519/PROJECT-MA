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
	TSubclassOf<UGameplayEffect> GetBaseCooldownEffect() const;
	
	const FSkillData& GetSkillData() const {return CachedSkillData;}
	const FModuleBehaviorData& GetBehaviorData() const {return CachedBehaviorData;}
	const FModuleElementalData& GetElementalData() const {return CachedElementalData;}
	const FModuleUtilityData& GetUtilityData() const {return CachedUtilityData;}
	const FModuleBehaviorData& GetComboData() const {return CachedComboData;}
	
	UFUNCTION()
	float GetTotalAnimSpeed() const;
	
	void ApplyDamageToHitResults(const TArray<FHitResult>& HitResults, float DamageMultiplier = 1.f);
	void ApplyDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetData, float DamageMultiplier =1.f);
	void ExecuteSkillAction(FGameplayEventData& Payload, float BehaviorMultiplier = 1.f);
	

	UFUNCTION(BlueprintCallable)
	FName GetSkillID() const {return SkillID;}
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
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
	UPROPERTY(Transient)
	FModuleBehaviorData CachedComboData;

	UPROPERTY(EditDefaultsOnly, Category="Config")
	FName SkillID;

	void PerformMeleeAttack(FGameplayEventData& Payload, float FinalMultiplier);
	void SpawnProjectile(FGameplayEventData& Payload, float DamageMultiplier);
	void SpawnTargetingProjectile(FGameplayEventData& Payload, float DamageMultiplier);
	AActor* SpawnProjectileActor(TSubclassOf<AActor> Class, FVector Loc, FRotator Rot, float DamageMultiplier, float ExplodeRadius = 0.f, bool bIsPenetrating=false);
	bool LoadSkillData();

	FGameplayEffectSpecHandle MakeSkillDamageSpec(float BehaviorMultiplier);

	void ApplyHitStop(AActor* TargetActor);
	
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

	UFUNCTION()
	void HandleProjectileHit(AActor* HitActor);

	UPROPERTY(Transient)
	float CurrentReactDuration = 0.f;
	
public:
	void Montage_SetPlayRate(UAnimMontage* AnimMontage, float PlayRate);
	void Montage_SetSection(FName SectionName);
	void SetHitReactionTag(FGameplayTag NewTag) {CachedSkillData.HitReactionTag = NewTag;}

	void SetReactDuration(float NewDuration) {CurrentReactDuration = NewDuration;}
	float GetReactDuration() const {return CurrentReactDuration;}
	
	bool TryActivateComboModule();
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;

	UPROPERTY()
	float ChargeRatio = 1.f;
};
