// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MASkillBehavior.h"
#include "GameplayTagContainer.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/MASkillVFXSet.h"
#include "MAGameplayAbility_SkillBase.generated.h"

class UMAAbilitySystemComponent;
class UUtilityModule;
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
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	FORCEINLINE FGameplayTag GetSharedCooldownTag() const {return CooldownTag;}
	FORCEINLINE FGameplayTag GetSkillElementTag() const {return ActiveElementTag;}
	FORCEINLINE FGameplayTag GetVFXRootTag() const {return VFXEventRootTag;}
	FORCEINLINE TObjectPtr<UUtilityModule> GetActiveUtilityModule() const {return ActiveUtilityModule;}

	TSubclassOf<UGameplayEffect> GetBaseDamageEffect() const;
	TSubclassOf<UGameplayEffect> GetBaseCooldownEffect() const;
	UDataTable* GetElementDataTable() const;
	/***************************************************************/
	/*						Skill Module						   */
	/***************************************************************/
private:
	//속성 모듈
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTag ActiveElementTag;
	
	//유틸리티 모듈
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveUtilityTag;
	UPROPERTY()
	TObjectPtr<UUtilityModule> ActiveUtilityModule;
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UUtilityModule>> CachedUtilityModules;

	//행동 모듈
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveBehaviorTag;
	UPROPERTY()
	TObjectPtr<UMASkillBehavior> ActiveBehaviorModule;
	
	FGameplayTag CooldownDurationTag;
	FGameplayTag ElementalModifierTag;
	FGameplayTag BehaviorModifierTag;
	FGameplayTag CooldownTag;
	FGameplayTag VFXEventRootTag;
	
	void ApplyBehaviorCooldown(float Multiplier);
	bool bCooldownApplied = false;
	
	FName BPName;
public:
	void ApplyGESpecToOwner(FGameplayEffectSpecHandle SpecHandle);
	const F_ElementInfoRow* GetActiveElementInfoRow();
	void ApplyDamageToHitResults(const TArray<FHitResult>& HitResults);
	void ApplyDamageToTargetData(const FGameplayAbilityTargetDataHandle& TargetData);
	UFUNCTION(BlueprintCallable)
	void ApplyDefaultCooldownOnce();
	UFUNCTION(BlueprintCallable)
	void ApplyShortCooldownAndRequestEndAbility();
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
};
