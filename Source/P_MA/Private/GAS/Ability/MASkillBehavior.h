// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "MASkillBehavior.generated.h"

class UMAGameplayAbility_SkillBase;
class AMACharacter;
class UAnimMontage;
/**
 * 
 */
UCLASS(Blueprintable, Abstract, EditInlineNew)
class UMASkillBehavior : public UObject
{
	GENERATED_BODY()

public:
	//이 클래스(모듈)을 사용하는 스킬의 참조
	UPROPERTY()
	TObjectPtr<UMAGameplayAbility_SkillBase> OwningAbility;
	//BlueprintNativeEvent : 블루프린트에서 C++내용 오버라이드 가능하도록
	//스킬 활성화시 호출될 내용 - OnActivate : 호출용(override X) / OnActivate_Implementation : 구현용(override O)
	UFUNCTION(BlueprintNativeEvent, Category="Skill Behavior")
	void OnActivate();
	virtual void OnActivate_Implementation();
	//스킬 종료시 호출될 내용
	UFUNCTION(BlueprintNativeEvent, Category="Skill Behavior")
	void OnEndAbility();
	virtual void OnEndAbility_Implementation();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	//입력 필요한 스킬인지
	virtual bool IsRequirePlayerInput() const {return false;}
	//스킬 사용 중 캐릭터 회전 막기
	virtual bool ShouldLockRotation() const {return true;}

protected:
	//자식 클래스가 캐릭터 접근 쉽게 하도록 돕는 헬퍼
	class AMACharacter* GetCharacter() const;
	UPROPERTY()
	TObjectPtr<class AMACharacter> Character;
	UPROPERTY()
	TObjectPtr<class AMAPlayerCharacter> PlayerCharacter;
};
