// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UtilityModule.generated.h"

struct FGameplayEffectSpecHandle;
class UMAGameplayAbility_SkillBase;
/**
 * 
 */
UCLASS(Blueprintable, Abstract, EditInlineNew)
class UUtilityModule : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UMAGameplayAbility_SkillBase> OwningAbility;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Skill Utility Module")
	void OnAbilityActivate();
	virtual void OnAbilityActivate_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Skill Utility Module")
	void OnAbilityEnd(bool bWasCancelled);
	virtual void OnAbilityEnd_Implementation(bool bWasCancelled);

	//데미지 수정하는 모듈에 사용
	virtual void ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const {};
	//쿨다운 수정하는 모듈에 사용
	virtual float ModifyCooldownDuration(float OriginalDuration) const {return OriginalDuration;}
	//애니메이션 속도 수정하는 모듈에 사용
	virtual float ModifyMontagePlayRate(float OriginalPlayRate) const {return OriginalPlayRate;}
};
