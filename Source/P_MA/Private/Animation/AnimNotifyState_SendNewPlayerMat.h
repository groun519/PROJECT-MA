// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "AnimNotifyState_SendNewPlayerMat.generated.h"


/**
 * 
 */
UCLASS()
class UAnimNotifyState_SendNewPlayerMat : public UAnimNotifyState
{
	GENERATED_BODY()
		
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	/** Body **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material|Body")
	FMaterialParamData BodyParam;
	
	/** Eye **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material|Eye")
	FMaterialParamData EyeParam;
};
