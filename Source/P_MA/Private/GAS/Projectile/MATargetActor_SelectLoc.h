// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "MATargetActor_SelectLoc.generated.h"

class UMaterialParameterCollection;
/**
 * 
 */
UCLASS()
class AMATargetActor_SelectLoc : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMATargetActor_SelectLoc();
	
	void SetTargetAreaRadius(float NewRadius);
	FORCEINLINE void SetTargetTraceRange(float NewRange) {Distance=NewRange;}
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	
private:
	virtual void Tick(float DeltaTime) override;
	virtual void ConfirmTargetingAndContinue() override;
	
	
	UPROPERTY(EditDefaultsOnly)
	float TargetAreaRadius = 300.f;
	UPROPERTY(EditDefaultsOnly)
	float Distance = 2000.f;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Visual")
	class UDecalComponent* SkillLocDecal;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DecalDMI;
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor InRangeColor = FLinearColor(0.700000,2.600000,5.000000,1.000000);
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FLinearColor OutOfRangeColor = FLinearColor::Red;
	
	FVector GetTargetPoint() const;
};
