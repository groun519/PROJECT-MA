#include "AI/Golem/GameplayAbility_ChargeSmash.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/MAAttributeSet.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UGameplayAbility_ChargeSmash::UGameplayAbility_ChargeSmash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGameplayAbility_ChargeSmash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		const float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());
		if (Fury < 15.f)
		{
			return false;
		}
	}

	return true;
}

void UGameplayAbility_ChargeSmash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		const float Fury = ASC->GetNumericAttribute(UMAAttributeSet::GetFuryAttribute());
		if (Fury < 15.f)
		{
			K2_EndAbility();
			return;
		}
	}

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !ChargeSmashMontage)
	{
		K2_EndAbility();
		return;
	}
	
	if (UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance())
	{
	}
	else
	{
		K2_EndAbility();
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeSmashMontage, 1.0f);
	if (!MontageTask)
	{
		K2_EndAbility();
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_ChargeSmash::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_ChargeSmash::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_ChargeSmash::OnMontageCompleted);
	MontageTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetChargeSmashTag());
	if (WaitLaunchEventTask)
	{
		WaitLaunchEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_ChargeSmash::StartCharging);
		WaitLaunchEventTask->ReadyForActivation();
	}
}

void UGameplayAbility_ChargeSmash::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

FGameplayTag UGameplayAbility_ChargeSmash::GetChargeSmashTag() const
{
	return FGameplayTag::RequestGameplayTag(FName("Ability.Monster.ChargeSmash"));
}

void UGameplayAbility_ChargeSmash::StartCharging(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		
	}
}

void UGameplayAbility_ChargeSmash::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{

	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor);
	if (Character)
	{
		if (UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance())
		{
			if (ChargeSmashMontage && AnimInst->Montage_IsPlaying(ChargeSmashMontage))
			{
				return;
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
