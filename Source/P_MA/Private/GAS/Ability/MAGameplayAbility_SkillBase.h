// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MASkillBehavior.h"
#include "GameplayTagContainer.h"
#include "GAS/MAGameplayAbility.h"
#include "MAGameplayAbility_SkillBase.generated.h"

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

	FORCEINLINE FGameplayTag GetSharedCooldownTag() const {return SharedCooldownTag;}
	FORCEINLINE FGameplayTag GetSkillElementTag() const {return ActiveSkillElementTag;}
	FORCEINLINE FGameplayTag GetVFXRootTag() const {return VFXEventRootTag;}
	FORCEINLINE UDataTable* GetElementDataTable() const {return ElementDataTable;}

	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	
	/***************************************************************/
	/*						Skill Module						   */
	/***************************************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag DefaultElementTag = FGameplayTag::RequestGameplayTag("Ability.Attribute.Default");
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag DefaultUtilityTag = FGameplayTag::RequestGameplayTag("Ability.Utility.Smite");
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag DefaultBehaviorTag = FGameplayTag::RequestGameplayTag("Ability.Behavior.Attack.Default");
	
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	TSubclassOf<UGameplayEffect> CooldownGE;
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag SharedCooldownTag;
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	TObjectPtr<UDataTable> ElementDataTable;
	
	//속성 모듈
	UPROPERTY(BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	FGameplayTag ActiveSkillElementTag;
	
	//유틸리티 모듈
	UPROPERTY(BlueprintReadOnly, Category="Utility Module", meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveUtilityTag;
	UPROPERTY()
	TObjectPtr<UUtilityModule> ActiveUtilityModule;
	
	//스킬 행동 모듈
	UPROPERTY(EditDefaultsOnly, Category="Behavior Module", Instanced)
	TMap<FGameplayTag,TObjectPtr<UMASkillBehavior>> BehaviorModules;
	UPROPERTY(BlueprintReadOnly, Category="Behavior Module", meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveBehaviorTag;
	UPROPERTY()
	TObjectPtr<UMASkillBehavior> ActiveBehaviorModule;
	
	
	FGameplayTag CooldownDurationTag;
	FGameplayTag VFXEventRootTag;
	
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
