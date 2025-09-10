// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SendNewPlayerTrans.generated.h"

UENUM(BlueprintType)
enum class EMoveType : uint8
{
	None,
	Launch,
	Teleport,
};
/**
 * 
 */
UCLASS()
class UAnimNotify_SendNewPlayerTrans : public UAnimNotify
{
	GENERATED_BODY()
		
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMoveType MoveType = EMoveType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="MoveType==EMoveType::Launch"))
	float LaunchPower = 100.f;
};
