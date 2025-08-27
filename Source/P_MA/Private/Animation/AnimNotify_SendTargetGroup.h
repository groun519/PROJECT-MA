// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_SendTargetGroup.generated.h"

/**
 * 
 */
UCLASS()
class UAnimNotify_SendTargetGroup : public UAnimNotify
{
	GENERATED_BODY()
		
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TArray<FName> TargetSocketNames;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bUsingfirstSocketOnly = false;
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	float Height = 5.0f;
};
