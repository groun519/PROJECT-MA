#include "AI/Golem/GA_ChargeSmash.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogGAChargeSmash, Log, All);

UGA_ChargeSmash::UGA_ChargeSmash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_ChargeSmash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		UE_LOG(LogGAChargeSmash, Warning, TEXT("[GA_ChargeSmash] Commit 실패"));
		K2_EndAbility();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character || !ChargeSmashMontage)
	{
		UE_LOG(LogGAChargeSmash, Warning, TEXT("[GA_ChargeSmash] 캐릭터 또는 몽타주 없음"));
		K2_EndAbility();
		return;
	}

	// ✅ 1. 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeSmashMontage, 1.f);
	MontageTask->OnCompleted.AddDynamic(this, &UGA_ChargeSmash::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_ChargeSmash::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_ChargeSmash::OnMontageCompleted);
	MontageTask->ReadyForActivation();

	UE_LOG(LogGAChargeSmash, Log, TEXT("[GA_ChargeSmash] Montage 재생 시작: %s"), *ChargeSmashMontage->GetName());

	// ✅ 2. Ability.End 태그 대기 (AnimNotify에서 송출)
	UAbilityTask_WaitGameplayEvent* WaitEndEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Change.End")));
	WaitEndEvent->EventReceived.AddDynamic(this, &UGA_ChargeSmash::OnEndEventReceived);
	WaitEndEvent->ReadyForActivation();
}

void UGA_ChargeSmash::OnEndEventReceived(FGameplayEventData Payload)
{
	UE_LOG(LogGAChargeSmash, Log, TEXT("[GA_ChargeSmash] Ability.Combo.Change.End 태그 수신 → Ability 종료"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ChargeSmash::OnMontageCompleted()
{
	UE_LOG(LogGAChargeSmash, Log, TEXT("[GA_ChargeSmash] Montage 완료"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_ChargeSmash::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogGAChargeSmash, Log, TEXT("[GA_ChargeSmash] Ability 종료 (Cancelled=%d)"), bWasCancelled);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
