// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MASkillBehavior.h"
#include "GameplayTagContainer.h"
#include "GAS/MAGameplayAbility.h"
#include "MAGameplayAbility_SkillBase.generated.h"

/**
 * 
 */
UCLASS()
class UMAGameplayAbility_SkillBase : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UMAGameplayAbility_SkillBase();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	FORCEINLINE FGameplayTag GetSharedCooldownTag() const {return SharedCooldownTag;}
	FORCEINLINE FGameplayTag GetSkillElementTag() const {return SkillElementTag;}
	FORCEINLINE FGameplayTag GetVFXRootTag() const {return VFXEventRootTag;}
	FORCEINLINE UDataTable* GetElementDataTable() const {return ElementDataTable;}

	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	
	/***************************************************************/
	/*						Skill Module						   */
	/***************************************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag SkillBehaviorTag = FGameplayTag::RequestGameplayTag("Ability.Behavior.Attack.Default");
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag SkillElementTag = FGameplayTag::RequestGameplayTag("Ability.Attribute.Default");
	//Default행동의 쿨다운
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	TSubclassOf<UGameplayEffect> CooldownGE;
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag CooldownDurationTag = FGameplayTag::RequestGameplayTag("Data.Cooldown.Duration");
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag SharedCooldownTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	TObjectPtr<UDataTable> ElementDataTable;
	/*
	// 스킬 사용 시 짧은 버프를 부여하는 모듈
	UPROPERTY(EditDefaultsOnly, Category="Utility Module")
	TArray<TSubclassOf<UGameplayEffect>> ModuleUtility;
	*/
	
	//스킬 행동 변경 모듈
	UPROPERTY(EditDefaultsOnly, Category="Behavior Module", Instanced)
	TMap<FGameplayTag,TObjectPtr<UMASkillBehavior>> BehaviorModules;
	UPROPERTY(BlueprintReadOnly, Category="Skill Behavior", meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveBehaviorTag;
	UPROPERTY()
	TObjectPtr<UMASkillBehavior> ActiveSkillBehavior;

	
	FGameplayTag VFXEventRootTag = FGameplayTag::RequestGameplayTag("Event.VFX");
	
	void ApplyBehaviorCooldown(float Multiplier);
	bool bCooldownApplied = false;
public:
	void ApplyDamageToHitResults(const TArray<FHitResult>& HitResults, TSubclassOf<UGameplayEffect> DamageEffect);
	void ApplyDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetData, TSubclassOf<UGameplayEffect> DamageEffect);
	UFUNCTION(BlueprintCallable)
	void ApplyDefaultCooldownOnce();
	UFUNCTION(BlueprintCallable)
	void ApplyShortCooldownAndRequestEndAbility();
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
};
