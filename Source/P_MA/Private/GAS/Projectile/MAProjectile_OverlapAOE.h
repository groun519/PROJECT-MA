// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MABaseProjectile.h"
#include "MAProjectile_OverlapAOE.generated.h"

/**
 * 앞으로 날아가는 투사체
 */
UCLASS()
class AMAProjectile_OverlapAOE : public AMABaseProjectile
{
	GENERATED_BODY()

public:
	AMAProjectile_OverlapAOE();

	//폭발 데미지 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability", meta=(ExposeOnSpawn="true"))
	float ImpactRadius = 300.f;

protected:
	virtual void SetupCollision() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnOverlapPawn(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	
	void Explode(FVector Location, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, Category="Ability")
	float InitSpeed = 800.f;
	
};
