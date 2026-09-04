// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlphaBlend.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "MAAnimInstance.generated.h"

class UAnimSequence;
class UAnimSequenceBase;
class UAnimMontage;
class UMASkillAbility;

/**
 * 
 */
UCLASS()
class UMAAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const { return Speed != 0; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsNotMoving() const { return Speed == 0; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetLookYawOffset() const { return LookRotOffset.Yaw; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetLookPitchOffset() const { return LookRotOffset.Pitch; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsOnGround() const { return !bIsJumping; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMounted() const { return bIsMounted; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE UAnimSequence* GetCurrentRideSequence() const { return CurrentRideSequence; }

	void SetCurrentRideSequence(UAnimSequence* InRideSequence) { CurrentRideSequence = InRideSequence; }
	void RegisterAnimationOwner(const UAnimSequenceBase* Animation, UMASkillAbility* SkillAbility);
	UMASkillAbility* FindAnimationOwner(const UAnimSequenceBase* Animation) const;
	void UnregisterAnimationOwner(const UAnimSequenceBase* Animation, const UMASkillAbility* SkillAbility);
	void RegisterSkillAreaPreviewContext(const UAnimSequenceBase* Animation, float AreaScale, FGameplayTag VisualTag);
	bool FindSkillAreaPreviewContext(const UAnimSequenceBase* Animation, float& OutAreaScale, FGameplayTag& OutVisualTag) const;
	void UnregisterSkillAreaPreviewContext(const UAnimSequenceBase* Animation);

	/** Captures the current pose and starts recovery. Completion is invoked only after a started recovery ends. */
	bool RecoverPose(FSimpleDelegate OnCompleted);
	/** Cancels the active recovery and discards its pending completion. */
	void CancelPoseRecovery();
	bool IsPoseRecoveryActive() const { return ActiveRecoveryMontage != nullptr; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	float GetRecoveryPoseAlpha() const { return RecoveryPoseAlpha; }


	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetVerticalInput() const
	{
		if (Velocity.IsNearlyZero())
			return 0.f;

		const FVector Forward = BodyPrevRot.Vector();
		return FVector::DotProduct(Velocity, Forward); // Speed 포함
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetHorizontalInput() const
	{
		if (Velocity.IsNearlyZero())
			return 0.f;

		const FVector Right = FRotationMatrix(BodyPrevRot).GetScaledAxis(EAxis::Y);
		return FVector::DotProduct(Velocity, Right); // Speed 포함
	}

private:
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	UPROPERTY()
	class UCharacterMovementComponent* OwnerMovementComp;

	FVector Velocity;
	float Speed;
	FRotator BodyPrevRot;
	FRotator LookRotOffset;
	bool bIsJumping;
	bool bIsMounted = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> CurrentRideSequence = nullptr;

	TMap<TObjectPtr<const UAnimSequenceBase>, TWeakObjectPtr<UMASkillAbility>> AnimationOwners;
	TMap<TObjectPtr<const UAnimSequenceBase>, float> SkillAreaPreviewScales;
	TMap<TObjectPtr<const UAnimSequenceBase>, FGameplayTag> SkillAreaPreviewVisualTags;

	/** Pose Recovery **/
	void HandleRecoveryMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void ResetRecoveryPoseBlend();

	UPROPERTY(EditDefaultsOnly, Category = "Recovery")
	TObjectPtr<UAnimSequenceBase> RecoveryAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Recovery", meta = (ClampMin = "0.1", Units = "s"))
	float RecoveryDuration = 1.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Recovery", meta = (ClampMin = "0.0", Units = "s"))
	float RecoveryPoseBlendDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Recovery", meta = (ClampMin = "0.0", Units = "s"))
	float RecoveryAnimationBlendOutDuration = 0.1f;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveRecoveryMontage;

	FSimpleDelegate RecoveryCompletedDelegate;
	FAlphaBlend RecoveryPoseBlend;
	float RecoveryPoseAlpha = 0.f;
};
