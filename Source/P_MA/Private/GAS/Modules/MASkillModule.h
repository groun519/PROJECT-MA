// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MASkillModule.generated.h"

class UMAGameplayAbility_Skill;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class UMASkillModule : public UObject
{
	GENERATED_BODY()

public:
	// 초기화
	virtual void InitializeModule(class UMAGameplayAbility_Skill* InSkill){OwnerSkill = InSkill;}
	// 활성화
	virtual void OnAbilityActivated() {}
	// 이벤트 수신
	virtual void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& Payload) {}
	// 종료
	virtual void OnAbilityEnded(bool bWasCancelled) {}

	// 상태이상 부여
	virtual void CreateAdditionalEffectSpecs(TArray<FGameplayEffectSpecHandle>& OutAdditionalSpecs) const {}
	// 수치 조정
	virtual void ModifyDamageSpec(FGameplayEffectSpecHandle& SpecHandle) const {}
	// 쿨타임 조정
	virtual void ModifyCooldownSpec(FGameplayEffectSpecHandle& SpecHandle) const {}
	// 애니메이션 속도 조정
	virtual float GetAnimSpeedMultiplier() const {return 1.0f;}
	
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UMAGameplayAbility_Skill> OwnerSkill;
};