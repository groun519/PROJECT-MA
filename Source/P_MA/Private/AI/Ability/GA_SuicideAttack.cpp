// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Ability/GA_SuicideAttack.h"
#include "AI/Golem/Monster.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

void UGA_SuicideAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	IgnoreTargets.Empty();

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Monster = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Monster || !SuicideMontage)
	{
		K2_EndAbility();
		return;
	}

	// 반복 거리 체크 시작
	UAbilityTask_WaitDelay* DistanceCheckTask = UAbilityTask_WaitDelay::WaitDelay(this, CheckInterval);
	DistanceCheckTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnDistanceCheckTick);
	DistanceCheckTask->ReadyForActivation();
}

void UGA_SuicideAttack::OnDistanceCheckTick()
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Monster = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!Monster)
	{
		K2_EndAbility();
		return;
	}

	AAIController* AICon = Cast<AAIController>(Monster->GetController());
	AActor* Target = nullptr;

	if (AICon && AICon->GetBlackboardComponent())
	{
		Target = Cast<AActor>(AICon->GetBlackboardComponent()->GetValueAsObject("Target"));
	}

	// 타겟 없음 → 계속 체크
	if (!Target)
	{
		UAbilityTask_WaitDelay* LoopTask = UAbilityTask_WaitDelay::WaitDelay(this, CheckInterval);
		LoopTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnDistanceCheckTick);
		LoopTask->ReadyForActivation();
		return;
	}

	// 거리 계산
	FVector ML = Monster->GetActorLocation();
	FVector TL = Target->GetActorLocation();
	ML.Z = TL.Z = 0;

	const float RawDist = FVector::Dist(ML, TL);

	float CapsuleOffset = 0.f;
	if (Monster->GetCapsuleComponent() && Target->GetRootComponent())
	{
		if (auto* MCap = Monster->GetCapsuleComponent())
		{
			if (auto* TCap = Cast<UCapsuleComponent>(Target->GetRootComponent()))
			{
				CapsuleOffset = MCap->GetScaledCapsuleRadius() + TCap->GetScaledCapsuleRadius();
			}
		}
	}

	const float VisualDist = RawDist - CapsuleOffset;

	// ★ 트리거 범위 진입 → 폭발 모션 + 데미지 이벤트 대기
	if (VisualDist <= TriggerRange)
	{
		UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance();
		if (!Anim)
		{
			K2_EndAbility();
			return;
		}

		// 몽타주 재생
		auto* PlayTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, SuicideMontage);

		PlayTask->OnCompleted.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);
		PlayTask->OnCancelled.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);
		PlayTask->OnInterrupted.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);
		PlayTask->OnBlendOut.AddDynamic(this, &UGA_SuicideAttack::K2_EndAbility);

		PlayTask->ReadyForActivation();

		// ★ DamageEvent 대기
		auto* DamageEvent =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Damage")));

		DamageEvent->EventReceived.AddDynamic(this, &UGA_SuicideAttack::OnDamageEvent);
		DamageEvent->ReadyForActivation();

		return;
	}

	// 계속 체크 루프
	UAbilityTask_WaitDelay* LoopTask =
		UAbilityTask_WaitDelay::WaitDelay(this, CheckInterval);

	LoopTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnDistanceCheckTick);
	LoopTask->ReadyForActivation();
}

void UGA_SuicideAttack::OnDamageEvent(FGameplayEventData Data)
{
	TArray<FHitResult> Hits = GetHitResultFromVirtualSocketTargetData(Data.TargetData);

	for (const FHitResult& Hit : Hits)
	{
		if (IgnoreTargets.Contains(Hit.GetActor()))
			continue;

		ApplyGameplayEffectToHitResultActor(
			Hit,
			DamageEffect,
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
		);

		IgnoreTargets.Add(Hit.GetActor());
	}

	// ★★★ Destroy() 제거 ★★★
	// 몬스터는 Barrack 풀로 되돌아가야 함

	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		if (AMonster* Monster = Cast<AMonster>(CurrentActorInfo->AvatarActor.Get()))
		{
			Monster->Deactivate();   // Barrack 풀 방식
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
