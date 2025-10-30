// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_ChargeExpandTrace.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChargeValueChanged, float, NewChargeRatio);
class AGameplayAbilityTargetActor;
/**
 * 
 */
UCLASS()
class USkillBehavior_ChargeExpandTrace : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;

	virtual bool IsRequirePlayerInput() const override {return true;}
	virtual bool ShouldLockRotation() const override {return false;}

	FOnChargeValueChanged OnChargeValueChanged;
protected:

	/** 최대 차지 시간 */
	UPROPERTY(EditDefaultsOnly)
	float MaxChargeDuration = 2.0f;
	UPROPERTY(EditDefaultsOnly)
	float SkillTimeoutDuration = 4.0f;

	/** 재생할 시각 효과 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> NiagaraEffect;
	
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "TraceShape == ETraceShape::Sphere", EditConditionHides))
	TSubclassOf<class AMATargetActor_ImedDamage> SphereIndicatorClass;
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "TraceShape == ETraceShape::Box", EditConditionHides))
	TSubclassOf<class AMATargetActor_ImedDamage> BoxIndicatorClass;

	/** 트레이스 모양 */
	UPROPERTY(EditDefaultsOnly)
	ETraceShape TraceShape = ETraceShape::Box;

	/** Box 트레이스 설정 */
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "TraceShape == ETraceShape::Box", EditConditionHides))
	float MinTraceDistance = 200.f;
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "TraceShape == ETraceShape::Box", EditConditionHides))
	float MaxTraceDistance = 1000.f;
	
	/** Sphere 트레이스 반지름 설정 */
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "TraceShape == ETraceShape::Sphere", EditConditionHides))
	float MinTraceRadius = 50.f;
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "TraceShape == ETraceShape::Sphere", EditConditionHides))
	float MaxTraceRadius = 150.f;
	
	/** 기본 트레이스 폭/넓이 */
	UPROPERTY(EditDefaultsOnly, meta=(EditCondition = "TraceShape == ETraceShape::Box", EditConditionHides))
	float FixedTraceRadius = 100.f;
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "TraceShape == ETraceShape::Box", EditConditionHides))
	float FixedTraceHalfHeight = 0.f;
	
private:
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> WaitInputReleaseTask;
	TWeakObjectPtr<class UAbilityTask_WaitDelay> SkillTimeoutTask;
	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask;
	FTimerHandle ChargeUpdateTimerHandle;

	//인디케이터 크기 업데이트 함수
	UFUNCTION()
	void UpdateChargeIndicator();
	//차징 시간 초과 -> 스킬 사용 취소
	UFUNCTION()
	void OnSkillTimeout();
	//키에서 손 뗌 -> 스킬 사용
	UFUNCTION()
	void OnChargeReleased(float TimeHeld);
	//마우스 클릭 -> 스킬 사용
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void ExecuteConfirmedBehavior(const FGameplayAbilityTargetDataHandle& Data);
	
	bool bIsChargeComplete = false;
	float ChargeStartTime = 0.f;
	float CurrentChargeRatio =0.f;
	float FinalEffectRadius = 0.f;
	float FinalEffectDistance = 0.f;
	float FinalEffectHalfHeight = 0.f;

	UPROPERTY()
	TObjectPtr<AMATargetActor_ImedDamage> ChargingRangeIndicator;
};
