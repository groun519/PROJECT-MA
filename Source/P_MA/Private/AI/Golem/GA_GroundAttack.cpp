#include "AI/Golem/GA_GroundAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogGAChargeSmash, Log, All);

UGA_GroundAttack::UGA_GroundAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_GroundAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
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

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeSmashMontage, 1.f);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_GroundAttack::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_GroundAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_GroundAttack::OnMontageCompleted);
	MontageTask->ReadyForActivation();
	
	// Ability.End 태그 대기
	UAbilityTask_WaitGameplayEvent* WaitEndEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Change.End")));
	WaitEndEvent->EventReceived.AddDynamic(this, &UGA_GroundAttack::OnEndEventReceived);
	WaitEndEvent->ReadyForActivation();

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboTargetEventTag());
		WaitTargetEventTask->EventReceived.AddDynamic(this, &UGA_GroundAttack::HitTarget);
		WaitTargetEventTask->ReadyForActivation();
	}
}

void UGA_GroundAttack::OnEndEventReceived(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GroundAttack::HitTarget(FGameplayEventData Data)
{
	TArray<FHitResult> HitResults =
		GetHitResultFromVirtualSocketTargetData(Data.TargetData);

	if (HitResults.Num() == 0)
	{
		UE_LOG(LogGAChargeSmash, Warning, TEXT("[ChargeSmash] No HitResults found in TargetData"));
		return;
	}

	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();

		if (!HitActor)
		{
			UE_LOG(LogGAChargeSmash, Warning, TEXT("[ChargeSmash] HitResult has no actor!"));
			continue;
		}

		UE_LOG(LogGAChargeSmash, Log, TEXT("[ChargeSmash] Hit Actor: %s  | Location: %s"),
			*HitActor->GetName(),
			*HitResult.ImpactPoint.ToString());

		// ASC(Ability System Component)가 존재하는지 확인
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);

		if (!TargetASC)
		{
			UE_LOG(LogGAChargeSmash, Warning,
				TEXT("[ChargeSmash] Hit Actor '%s' has NO AbilitySystemComponent! (Damage will NOT apply)"),
				*HitActor->GetName());
			continue;
		}

		// 이미 맞은 대상은 무시
		if (IgnoreTargets.Contains(HitActor))
		{
			UE_LOG(LogGAChargeSmash, Verbose,
				TEXT("[ChargeSmash] Hit Actor '%s' ignored (already hit)."),
				*HitActor->GetName());
			continue;
		}

		// 데미지 이펙트 적용
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectForCurrentCombo();
		ApplyGameplayEffectToHitResultActor(
			HitResult,
			GameplayEffect,
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
		);

		IgnoreTargets.Add(HitActor);

		UE_LOG(LogGAChargeSmash, Log,
			TEXT("[ChargeSmash] Applied DamageEffect '%s' to Actor '%s'"),
			*GetNameSafe(GameplayEffect),
			*HitActor->GetName());
	}
}


TSubclassOf<UGameplayEffect> UGA_GroundAttack::GetDamageEffectForCurrentCombo() const
{
	return DamageEffect;
}

void UGA_GroundAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GroundAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_GroundAttack::GetComboTargetEventTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.Combo.Damage");
}
