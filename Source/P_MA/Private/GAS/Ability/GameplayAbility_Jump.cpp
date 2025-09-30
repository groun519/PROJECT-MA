// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Jump.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimNotify_SendNewPlayerTrans.h"
#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"

UGameplayAbility_Jump::UGameplayAbility_Jump()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	if (!SkillMontage || !K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitStartJumpTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Start"));
	WaitStartJumpTask->EventReceived.AddDynamic(this, &UGameplayAbility_Jump::StartJumpEventReceived);
	WaitStartJumpTask->ReadyForActivation();
}


void UGameplayAbility_Jump::StartJumpEventReceived(FGameplayEventData Data)
{
	// 서버에서만 실제 이동 로직을 처리합니다.
	if (K2_HasAuthority())
	{
		ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (!Character) return;

		// 1. 캐릭터의 정면 방향으로 수평 속도를, 위쪽 방향으로 수직 속도를 계산합니다.
		const FVector ForwardVector = Character->GetActorForwardVector();
		const FVector JumpVelocity = (ForwardVector * JumpXYVelocity) + (FVector::UpVector * JumpZVelocity);

		// 2. 계산된 점프 속도를 담아 GAP_Movement에 "점프 실행" 이벤트를 보냅니다.
		FGameplayTag JumpActivateTag = FGameplayTag::RequestGameplayTag("Ability.Passive.Jump.Activate");
		PushTarget(Character, JumpVelocity, JumpActivateTag);
	}
}

/*
void UGameplayAbility_Jump::StartJumpEventReceived(FGameplayEventData Data)
{
	const FJumpData* JumpData = nullptr;
	
	const FGameplayAbilityTargetData* TargetData = Data.TargetData.Data[0].Get();

	if (!JumpData && TargetData->GetScriptStruct() == FJumpData::StaticStruct())
		JumpData = static_cast<const FJumpData*>(TargetData);

	FVector		OwnerLocation		= JumpData->OwnerLocation;
	FRotator	OwnerRotation		= JumpData->OwnerRotation;
	float		StartToEndTime		= JumpData->StartToEndTime;
	float		JumpTimeRequired	= JumpData->JumpTimeRequired;

	UE_LOG(LogGameplayTags, Log, TEXT("OwnerLocation	: %f, %f, %f"), OwnerLocation.X, OwnerLocation.Y, OwnerLocation.Z);
	UE_LOG(LogGameplayTags, Log, TEXT("OwnerRotation	: %f, %f, %f"), OwnerRotation.Pitch, OwnerRotation.Yaw, OwnerRotation.Roll);
	UE_LOG(LogGameplayTags, Log, TEXT("StartToEndTime	: %f"), StartToEndTime);
	UE_LOG(LogGameplayTags, Log, TEXT("JumpTimeRequired : %f"), JumpTimeRequired);
	UE_LOG(LogGameplayTags, Log, TEXT("============================="));
}
*/
FGameplayTag UGameplayAbility_Jump::GetJumpTag(EMovementNotifyTags TagType)
{
	switch (TagType)
	{
	case EMovementNotifyTags::None:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Jump");
	case EMovementNotifyTags::Start:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Start");
	case EMovementNotifyTags::End:
		return FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.End");
	default:
		return FGameplayTag();
	}	
}
