#include "Animation/MAAnimInstance.h"

#include "Animation/PoseSnapshot.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/Components/ReadyRideComponent.h"

void UMAAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter->GetCharacterMovement();
	}
}

void UMAAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (RecoveryPoseAlpha > 0.f)
	{
		RecoveryPoseBlend.Update(DeltaSeconds);
		RecoveryPoseAlpha = RecoveryPoseBlend.GetBlendedValue();
		if (RecoveryPoseBlend.IsComplete())
		{
			ResetRecoveryPoseBlend();
		}
	}

	if (OwnerCharacter)
	{
		if (const AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(OwnerCharacter))
		{
			if (const UReadyRideComponent* ReadyRideComp = PlayerCharacter->GetReadyRideComponent();
				ReadyRideComp)
			{
				bIsMounted = ReadyRideComp->GetMountState() == ERideMountState::Mounted;
				const bool bUseRideMovementSource = ReadyRideComp->IsRiding();
				if (bUseRideMovementSource)
				{
					Velocity = ReadyRideComp->GetRideMoveVelocity();
					Speed = ReadyRideComp->GetRideMoveSpeed();
				}
				else
				{
					Velocity = OwnerCharacter->GetVelocity();
					Speed = Velocity.Length();
				}
			}
			else
			{
				bIsMounted = false;
				Velocity = OwnerCharacter->GetVelocity();
				Speed = Velocity.Length();
			}
		}
		else
		{
			bIsMounted = false;
			Velocity = OwnerCharacter->GetVelocity();
			Speed = Velocity.Length();
		}
		FRotator BodyRot = OwnerCharacter->GetActorRotation();
		BodyPrevRot = BodyRot;

		FRotator ControlRot = OwnerCharacter->GetBaseAimRotation();
		LookRotOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, BodyRot);
	}
	else
	{
		bIsMounted = false;
	}
}

void UMAAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{

}

void UMAAnimInstance::RegisterAnimationOwner(const UAnimSequenceBase* Animation, UMASkillAbility* SkillAbility)
{
	if (!Animation || !SkillAbility) return;
	AnimationOwners.FindOrAdd(Animation) = SkillAbility;
}

UMASkillAbility* UMAAnimInstance::FindAnimationOwner(const UAnimSequenceBase* Animation) const
{
	if (!Animation) return nullptr;

	const TWeakObjectPtr<UMASkillAbility>* FoundOwner = AnimationOwners.Find(Animation);
	return FoundOwner ? FoundOwner->Get() : nullptr;
}

void UMAAnimInstance::UnregisterAnimationOwner(const UAnimSequenceBase* Animation, const UMASkillAbility* SkillAbility)
{
	if (!Animation) return;

	const TWeakObjectPtr<UMASkillAbility>* FoundOwner = AnimationOwners.Find(Animation);
	if (!FoundOwner) return;
	if (SkillAbility && FoundOwner->Get() != SkillAbility) return;

	AnimationOwners.Remove(Animation);
}

void UMAAnimInstance::RegisterSkillAreaPreviewContext(
	const UAnimSequenceBase* Animation,
	float AreaScale,
	FGameplayTag VisualTag)
{
	if (!Animation) return;

	SkillAreaPreviewScales.FindOrAdd(Animation) = AreaScale;
	SkillAreaPreviewVisualTags.FindOrAdd(Animation) = VisualTag;
}

bool UMAAnimInstance::FindSkillAreaPreviewContext(
	const UAnimSequenceBase* Animation,
	float& OutAreaScale,
	FGameplayTag& OutVisualTag) const
{
	if (!Animation) return false;

	const float* FoundAreaScale = SkillAreaPreviewScales.Find(Animation);
	const FGameplayTag* FoundVisualTag = SkillAreaPreviewVisualTags.Find(Animation);
	if (!FoundAreaScale || !FoundVisualTag) return false;

	OutAreaScale = *FoundAreaScale;
	OutVisualTag = *FoundVisualTag;
	return true;
}

void UMAAnimInstance::UnregisterSkillAreaPreviewContext(const UAnimSequenceBase* Animation)
{
	if (!Animation) return;

	SkillAreaPreviewScales.Remove(Animation);
	SkillAreaPreviewVisualTags.Remove(Animation);
}

void UMAAnimInstance::RecoverPose(FSimpleDelegate OnCompleted)
{
	static const FName RecoveryPoseSnapshotName(TEXT("MARecoveryPose"));
	CancelPoseRecovery();

	const float AnimationLength = RecoveryAnimation ? RecoveryAnimation->GetPlayLength() : 0.f;
	if (!ensureMsgf(
		RecoveryAnimation && AnimationLength > UE_KINDA_SMALL_NUMBER,
		TEXT("Pose Recovery requires a valid Recovery Animation on %s."),
		*GetClass()->GetName()))
	{
		StopAllMontages(0.f);
		ResetRecoveryPoseBlend();
		OnCompleted.ExecuteIfBound();
		return;
	}

	SavePoseSnapshot(RecoveryPoseSnapshotName);
	const FPoseSnapshot* RecoveryPose = GetPoseSnapshot(RecoveryPoseSnapshotName);
	const bool bHasRecoveryPose = RecoveryPose && RecoveryPose->bIsValid;
	ensureMsgf(bHasRecoveryPose, TEXT("Failed to capture Pose Recovery snapshot on %s."), *GetName());

	StopAllMontages(0.f);
	const float SafeRecoveryDuration = FMath::Max(RecoveryDuration, 0.1f);
	static const FName FullBodySlotName(TEXT("Full"));
	ActiveRecoveryMontage = PlaySlotAnimationAsDynamicMontage(
		RecoveryAnimation,
		FullBodySlotName,
		0.f,
		FMath::Max(RecoveryAnimationBlendOutDuration, 0.f),
		AnimationLength / SafeRecoveryDuration);
	if (!ActiveRecoveryMontage)
	{
		ensureMsgf(false, TEXT("Failed to play Pose Recovery animation on %s."), *GetName());
		ResetRecoveryPoseBlend();
		OnCompleted.ExecuteIfBound();
		return;
	}

	RecoveryCompletedDelegate = MoveTemp(OnCompleted);
	FOnMontageEnded RecoveryMontageEndedDelegate;
	RecoveryMontageEndedDelegate.BindUObject(this, &UMAAnimInstance::HandleRecoveryMontageEnded);
	Montage_SetEndDelegate(RecoveryMontageEndedDelegate, ActiveRecoveryMontage);

	const float SafePoseBlendDuration = bHasRecoveryPose
		? FMath::Clamp(RecoveryPoseBlendDuration, 0.f, SafeRecoveryDuration)
		: 0.f;
	if (SafePoseBlendDuration <= UE_KINDA_SMALL_NUMBER)
	{
		ResetRecoveryPoseBlend();
		return;
	}

	RecoveryPoseBlend.SetBlendTime(SafePoseBlendDuration);
	RecoveryPoseBlend.SetBlendOption(EAlphaBlendOption::HermiteCubic);
	RecoveryPoseBlend.SetValueRange(1.f, 0.f);
	RecoveryPoseBlend.Reset();
	RecoveryPoseAlpha = 1.f;
}

void UMAAnimInstance::CancelPoseRecovery()
{
	UAnimMontage* RecoveryMontage = ActiveRecoveryMontage;
	ActiveRecoveryMontage = nullptr;
	RecoveryCompletedDelegate.Unbind();
	ResetRecoveryPoseBlend();

	if (RecoveryMontage && Montage_IsActive(RecoveryMontage))
	{
		Montage_Stop(0.f, RecoveryMontage);
	}
}

void UMAAnimInstance::HandleRecoveryMontageEnded(UAnimMontage* Montage, bool /*bInterrupted*/)
{
	if (Montage != ActiveRecoveryMontage) return;

	ActiveRecoveryMontage = nullptr;
	ResetRecoveryPoseBlend();
	FSimpleDelegate CompletedDelegate = MoveTemp(RecoveryCompletedDelegate);
	RecoveryCompletedDelegate.Unbind();
	CompletedDelegate.ExecuteIfBound();
}

void UMAAnimInstance::ResetRecoveryPoseBlend()
{
	RecoveryPoseAlpha = 0.f;
}
