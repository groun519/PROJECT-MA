// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_VolcanoEruption.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

UGameplayAbility_VolcanoEruption::UGameplayAbility_VolcanoEruption()
{
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_VolcanoEruption::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	FHitResult TraceHit;
	APlayerController* PlayerController = GetActorInfo().PlayerController.Get();
	if (PlayerController)
		PlayerController -> GetHitResultUnderCursor(ECC_Visibility,false, TraceHit);
	
	TargetLocation = TraceHit.ImpactPoint;
	
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayStabMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
		PlayStabMontageTask -> OnBlendOut.AddDynamic(this, &UGameplayAbility_VolcanoEruption::K2_EndAbility);
		PlayStabMontageTask -> OnCancelled.AddDynamic(this, &UGameplayAbility_VolcanoEruption::K2_EndAbility);
		PlayStabMontageTask -> OnInterrupted.AddDynamic(this, &UGameplayAbility_VolcanoEruption::K2_EndAbility);
		PlayStabMontageTask -> OnCompleted.AddDynamic(this, &UGameplayAbility_VolcanoEruption::K2_EndAbility);
		PlayStabMontageTask -> ReadyForActivation();
	}
	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitJumpEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Movement.Jump"));
		WaitJumpEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_VolcanoEruption::JumpToTarget);
		WaitJumpEventTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* WaitTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetVolcanoEruptionDamageTag());
		WaitTargetEventTask->EventReceived.AddDynamic(this, &UGameplayAbility_VolcanoEruption::DoDamage);
		WaitTargetEventTask->ReadyForActivation();
	}
}

FGameplayTag UGameplayAbility_VolcanoEruption::GetVolcanoEruptionDamageTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Skill.VolcanoEruption.Damage");
}

void UGameplayAbility_VolcanoEruption::DoDamage(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(EventData.TargetData, ETeamAttitude::Hostile, ShouldDrawDebug(), true);
		for (FHitResult& HitResult : HitResults)
		{
			ApplyGameplayEffectToHitResultActor(HitResult, SkillDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}
}

void UGameplayAbility_VolcanoEruption::JumpToTarget(FGameplayEventData EventData)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (OwnerCharacter)
	{
		const FVector StartLocation = OwnerCharacter -> GetActorLocation();

		FVector LaunchVelocity;
		bool bHaveSolution = UGameplayStatics::SuggestProjectileVelocity(
			this,
			LaunchVelocity,
			StartLocation,
			TargetLocation,
			1.0f, // 점프 시간 (조절 필요)
			0.0f,
			0.0f,
			ESuggestProjVelocityTraceOption::DoNotTrace
		);

		if (bHaveSolution)
			OwnerCharacter -> LaunchCharacter(LaunchVelocity, true, true);
	}
}


