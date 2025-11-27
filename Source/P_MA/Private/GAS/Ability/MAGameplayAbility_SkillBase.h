// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MASkillBehavior.h"
#include "GameplayTagContainer.h"
#include "GAS/MAGameplayAbility.h"
#include "GAS/MASkillVFXSet.h"
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
	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;

	FORCEINLINE FGameplayTag GetSharedCooldownTag() const {return SharedCooldownTag;}
	FORCEINLINE FGameplayTag GetSkillElementTag() const {return ActiveElementTag;}
	FORCEINLINE FGameplayTag GetVFXRootTag() const {return VFXEventRootTag;}
	FORCEINLINE TSubclassOf<UGameplayEffect> GetBaseDamageEffect() const {return CachedSkillTemplate->DamageGEClass;}
	FORCEINLINE TObjectPtr<UUtilityModule> GetActiveUtilityModule() const {return ActiveUtilityModule;}

	UDataTable* GetElementDataTable() const;
	
	/***************************************************************/
	/*						Skill Module						   */
	/***************************************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	FGameplayTag SharedCooldownTag;
	UPROPERTY(EditDefaultsOnly, Category="Skill Name")
	FName SkillName;
	
	UPROPERTY()
	TObjectPtr<UMASkillTemplate> CachedSkillTemplate;
	FGameplayTag CachedCooldownTag;
	
	//속성 모듈
	UPROPERTY(BlueprintReadOnly, Category = "Setup", meta = (AllowPrivateAccess = "true"))
	FGameplayTag ActiveElementTag;
	
	//유틸리티 모듈
	UPROPERTY(BlueprintReadOnly, Category="Utility Module", meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveUtilityTag;
	UPROPERTY()
	TObjectPtr<UUtilityModule> ActiveUtilityModule;
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UUtilityModule>> CachedUtilityModules;

	//행동 모듈
	UPROPERTY(BlueprintReadOnly, Category="Behavior Module", meta=(AllowPrivateAccess="true"))
	FGameplayTag ActiveBehaviorTag;
	UPROPERTY()
	TObjectPtr<UMASkillBehavior> ActiveBehaviorModule;
	
	FGameplayTag CooldownDurationTag;
	FGameplayTag ElementalModifierTag;
	FGameplayTag BehaviorModifierTag;
	FGameplayTag VFXEventRootTag;
	
	void ApplyBehaviorCooldown(float Multiplier);
	bool bCooldownApplied = false;
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
