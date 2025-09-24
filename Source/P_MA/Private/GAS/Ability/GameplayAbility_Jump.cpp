// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Jump.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimNotify_SendNewPlayerTrans.h"
#include "GAS/MAAbilitySystemStatics.h"

UGameplayAbility_Jump::UGameplayAbility_Jump()
{
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

	// 어빌리티 태스크를 사용하여 몽타주를 재생합니다.
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
	
	// 몽타주가 끝나거나, 취소되거나, 중단되면 어빌리티를 종료합니다.
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Jump::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetJumpTag(EMovementNotifyTags::Start), nullptr, false, false);
	WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_Jump::OnJumpEventReceived);
	WaitComboChangeEventTask->ReadyForActivation();
}

void UGameplayAbility_Jump::OnJumpEventReceived(FGameplayEventData Data)
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
