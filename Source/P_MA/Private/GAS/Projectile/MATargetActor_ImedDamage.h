// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MATargetActor_ImedDamage.generated.h"

/**
 * 
 */
UCLASS()
class AMATargetActor_ImedDamage : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMATargetActor_ImedDamage();

	//타격 범위 설정 함수
	void SetTargetAreaRadius(float NewRadius);
	//스킬 사거리 설정 함수
	FORCEINLINE void SetTargetDistanceRange(float NewRange) {Distance = NewRange;}
	//타격 대상 설정
	void SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy=true);
	//디버그 그리는지
	FORCEINLINE void SetShouldDrawDebug(bool bDrawDebug) {bShouldDrawDebug = bDrawDebug;}

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	ETraceShape TargetShape = ETraceShape::Box;

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void ConfirmTargetingAndContinue() override;
	
private:
	bool bShouldTargetEnemy = true;
	bool bShouldTargetFriendly = false;
	bool bShouldDrawDebug = false;

	FVector GetTargetPoint() const;
	
	//스킬 사거리 변수
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float Distance = 2000.f;
	//스킬 범위 변수
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius = 300.f;
	//데칼 컴포넌트
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	class UDecalComponent* DecalComp;

	UFUNCTION()
	void HandleChargeUpdated(float NewChargeRatio);
};
