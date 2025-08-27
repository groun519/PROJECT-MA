// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_WeaponSegmentEvents.generated.h"

class UWeaponComponent;

/**
 * 
 */
UCLASS()
class P_MA_API UAnimNotifyState_WeaponSegmentEvents : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
							 float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
							float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
						   const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTag AbilityEventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bUseBeginEvent = true;
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability", meta=(EditCondition="bUseBeginEvent"))
	FGameplayTag BeginEventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bUseEndEvent = true;
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability", meta=(EditCondition="bUseEndEvent"))
	FGameplayTag EndEventTag;

	UPROPERTY(EditAnywhere, Category="Interp", meta=(ClampMin="1", AllowPrivateAccess="true"))
	int32 InterpCount = 5;

	// 자른 구간 저장하는 배열
	TArray<float> InterpMontagePos;

	// Cache
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CachedOwner;
	
	// SocketLoc
	FVector PrevBase       = FVector::ZeroVector; // 직전 틱 Base 월드 위치
	FVector PrevTip        = FVector::ZeroVector; // 직전 틱 Tip  월드 위치
	
	// SendGameplayEvent
	void SendSegment(const FVector& Start, const FVector& End) const;
	bool SendCurrentLocalSegment() const;
};
