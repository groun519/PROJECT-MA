// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GAS/AbilityVFXsData.h"
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

private:
	/***************************************************************/
	/*							Common		  				       */
	/***************************************************************/

	UPROPERTY(EditDefaultsOnly, Category="Skill | Common")
	TObjectPtr<UAnimMontage> SkillAnimMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Skill | Common")
	TArray<TSubclassOf<UGameplayEffect>> EffectsToApply;
	
	UPROPERTY(EditDefaultsOnly, Category="Skill | Common")
	FGameplayTag DamageEventTag;

	/***************************************************************/
	/*						Skill Module						   */
	/***************************************************************/

	// 스킬 사용 시 짧은 버프를 부여하는 모듈
	UPROPERTY(EditDefaultsOnly, Category="Skill | Module")
	TArray<TSubclassOf<UGameplayEffect>> ModuleUtility;
	//스킬에 속성 부여하는 모듈 - 방송의 구체적 내용 (어떤 이펙트를 적용할까)
	UPROPERTY(EditDefaultsOnly, Category="Skill | Module")
	FGameplayTag ModuleAttributeTag;
public:
	//스킬 행동 변경 모듈
	UPROPERTY(EditDefaultsOnly, Category="Skill | Module")
	FGameplayTag ModuleBehaviorTag;

private:
	UPROPERTY(EditDefaultsOnly, Category="Skill | Module")
	FName LoopSection = FName("Loop");
    
	UPROPERTY(EditDefaultsOnly, Category="Skill | Module")
	FName EndSection = FName("End");

	UPROPERTY(EditDefaultsOnly, Category="Skill | Module")
	TObjectPtr<UAbilityVFXsData> AttributeEffects;

	/***************************************************************/
	/*						     Other			 			       */
	/***************************************************************/
	
	//스킬에 속성 부여 시 방송할 채널 (이펙트를 키거나 꺼라)
	UPROPERTY(EditDefaultsOnly, Category="Skill | Cue")
	FGameplayTag AttributeCueTag;

	virtual void HandleDefaultSkill();
	virtual void HandleChargeSkill();
	virtual void HandleChainSkill();
	virtual void HandleHoldingSkill();

	UFUNCTION()
	void OnChargeEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void OnMaxCharged();
	UFUNCTION()
	void OnChargeReleased(float Time);
	
	UFUNCTION()
	void OnMaxHold();
	UFUNCTION()
	void OnForwardPlay(FGameplayEventData EventData);
	UFUNCTION()
	void OnReversePlay(FGameplayEventData EventData);
	UFUNCTION()
	void OnHoldReleased(float Time);

	
	UPROPERTY(EditDefaultsOnly, Category="Skill")
	float MaxChargeDuration = 3.0f;
	UPROPERTY(EditDefaultsOnly, Category="Skill")
	float MaxHoldDuration = 3.0f;
	UPROPERTY(EditDefaultsOnly, Category="Skill")
	float ReverseSpeed = -2.f;

	void SetMontagePlayRate(float NewPlayRate);
	void MontageToOtherSection(FName SectionName);
	bool bIsEnd = false;
	bool bIsHoldEnd = false;
};
