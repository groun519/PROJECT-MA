// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_WeaponSegmentEvents.generated.h"

class UBladeComponent;

struct FTrackerKey
{
	TWeakObjectPtr<USkeletalMeshComponent> MeshComp;
	TWeakObjectPtr<UBladeComponent>       WeaponComp;
	const void*                            Notify = nullptr; // this

	bool operator==(const FTrackerKey& Other) const
	{
		return MeshComp == Other.MeshComp && WeaponComp == Other.WeaponComp && Notify == Other.Notify;
	}
};

FORCEINLINE uint32 GetTypeHash(const FTrackerKey& K)
{
	uint32 H = ::GetTypeHash(K.MeshComp);
	H = HashCombine(H, ::GetTypeHash(K.WeaponComp));
	H = HashCombine(H, PointerHash(K.Notify));
	return H;
}

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
	
	// Cache
	TWeakObjectPtr<AActor> CachedOwner;
	TWeakObjectPtr<UBladeComponent> CachedBladeComponent;
	FTrackerKey TrackerKey;

	// SendGameplayEvent
	void SendSegment(AActor* Owner, const FVector& Start, const FVector& End) const;
};
