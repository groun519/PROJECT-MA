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
	
	/***************************************************************/
	/*						Skill Module						   */
	/***************************************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FGameplayTag SharedCooldownTag;
	UPROPERTY()
	FGameplayTag VFXEventRootTag = FGameplayTag::RequestGameplayTag("Event.VFX");
	
	UPROPERTY(EditDefaultsOnly, Category="Element")
	FGameplayTag SkillElementTag = FGameplayTag::RequestGameplayTag("Ability.Attribute.Default");
	UPROPERTY(EditDefaultsOnly, Category="Element")
	TObjectPtr<UDataTable> ElementDataTable;
	/*
	// 스킬 사용 시 짧은 버프를 부여하는 모듈
	UPROPERTY(EditDefaultsOnly, Category="Module")
	TArray<TSubclassOf<UGameplayEffect>> ModuleUtility;
	*/
	
	//스킬 행동 변경 모듈
	UPROPERTY(EditDefaultsOnly, Category="Module", Instanced)
	TMap<FGameplayTag,TObjectPtr<UMASkillBehavior>> BehaviorModules;

	// 동적 태그가 없을 때 사용할 기본 행동을 지정하는 태그
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DefaultBehaviorTag = FGameplayTag::RequestGameplayTag("Ability.Behavior.Attack.Default");
	
	UPROPERTY()
	TObjectPtr<UMASkillBehavior> ActiveSkillBehavior;

public:
	void SetMontagePlayRate(float NewPlayRate);
	void MontageToOtherSection(FName SectionName);
	void RequestEndAbility();
	void ApplyDamageToHitResults(const TArray<FHitResult>& HitResults, TSubclassOf<UGameplayEffect> DamageEffect);
	void ApplyDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetData, TSubclassOf<UGameplayEffect> DamageEffect);
	void ApplyEffectToOwner(TSubclassOf<UGameplayEffect> Effect, float Level = 1.0f);
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
};
