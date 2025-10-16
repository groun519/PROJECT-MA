// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "MATargetActor.generated.h"

/**
 * 
 */
UCLASS()
class AMATargetActor : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMATargetActor();
	
	void SetTargetAreaRadius(float NewRadius);
	void SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy=true);
	FORCEINLINE void SetTargetTraceRange(float NewRange) {Distance = NewRange;}
	FORCEINLINE void SetShouldDrawDebug(bool bDrawDebug) {bShouldDrawDebug = bDrawDebug;}

private:
	virtual void Tick(float DeltaTime) override;
	virtual void ConfirmTargetingAndContinue() override;

	UPROPERTY(EditDefaultsOnly)
	float Distance = 2000.f;
	UPROPERTY(EditDefaultsOnly)
	float TargetAreaRadius = 300.f;
	UPROPERTY(VisibleDefaultsOnly)
	class UDecalComponent* DecalComp;

	bool bShouldTargetEnemy = true;
	bool bShouldTargetFriendly = false;
	bool bShouldDrawDebug = false;

	FVector GetTargetPoint() const;
};
