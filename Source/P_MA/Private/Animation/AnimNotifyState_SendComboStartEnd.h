// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "AnimNotifyState_SendComboStartEnd.generated.h"

class UWeaponComponent;

/**
 * 
 */
UCLASS()
class P_MA_API UAnimNotifyState_SendComboStartEnd : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
							 float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
						   const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bUseBeginEvent = true;
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability", meta=(EditCondition="bUseBeginEvent"))
	FGameplayTag BeginEventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bUseEndEvent = true;
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability", meta=(EditCondition="bUseEndEvent"))
	FGameplayTag EndEventTag;
	UPROPERTY()
	FGameplayTag ClearEventTag = UMAAbilitySystemStatics::GetIgnoreClearTag();
};
