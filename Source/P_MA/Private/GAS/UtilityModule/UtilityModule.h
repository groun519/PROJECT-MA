// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UtilityModule.generated.h"

struct FSkillUtilityModule;
struct FGameplayEffectSpecHandle;
class UMAGameplayAbility_SkillBase;
/**
 * 
 */
UCLASS()
class UUtilityModule : public UObject
{
	GENERATED_BODY()

public:
	UUtilityModule();
	
	UPROPERTY()
	TObjectPtr<UMAGameplayAbility_SkillBase> OwningAbility;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Skill Utility Module")
	void OnAbilityActivate();
	void OnAbilityActivate_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Skill Utility Module")
	void OnAbilityEnd(bool bWasCancelled);
	void OnAbilityEnd_Implementation(bool bWasCancelled);

	//데미지 수정하는 모듈에 사용
	void ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const;
	//쿨다운 수정하는 모듈에 사용
	float ModifyCooldownDuration(float OriginalDuration) const;
	//애니메이션 속도 수정하는 모듈에 사용
	float ModifyMontagePlayRate(float OriginalPlayRate) const;

	void InitFromData(const FSkillUtilityModule& Data);
protected:
	FGameplayTag DamageModifierTag;

private:
	UPROPERTY()
	float DamagePercentAdditive = 0.f;
	
	UPROPERTY()
	float MontagePlayRate = 1.f;
	
	UPROPERTY()
	float CooldownMultiplier = 1.f;
	
	UPROPERTY(meta=(EditCondition="CooldownMultiplier == 0.f", EditConditionHides))
	float ChanceToReset=0.f;

	UPROPERTY()
	TSubclassOf<class UGameplayEffect> BuffGEOnActive;

	UPROPERTY()
	TSubclassOf<class UGameplayEffect> BuffGEOnEnd;
};
