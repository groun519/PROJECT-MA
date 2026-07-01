// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/Modules/MASkillModuleData.h"
#include "MAGameplayAbility_Skill.generated.h"

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
	FSkillData CachedSkillData;

	UPROPERTY(EditDefaultsOnly, Category="Config")
	FName SkillID;

	void PerformMeleeAttack(FGameplayEventData& Payload, float FinalMultiplier);
	void SpawnProjectile(FGameplayEventData& Payload, float DamageMultiplier);
	void SpawnTargetingProjectile(FGameplayEventData& Payload, float DamageMultiplier);
	bool LoadSkillData();

	FGameplayEffectSpecHandle MakeSkillDamageSpec(float BehaviorMultiplier);

	void ApplyHitStop(AActor* TargetActor);
	
public:
	void Montage_SetPlayRate(UAnimMontage* AnimMontage, float PlayRate);
	void Montage_SetSection(FName SectionName);

	UPROPERTY()
	TArray<AActor*> IgnoreTargets;

	UPROPERTY()
	float ChargeRatio = 1.f;
};
