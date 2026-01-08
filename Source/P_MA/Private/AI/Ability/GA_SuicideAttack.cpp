// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Ability/GA_SuicideAttack.h"

#include "AI/Golem/Monster.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

void UGA_SuicideAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	IgnoreTargets.Empty();
	bHasTriggeredExplosion = false;
	bKillDelayStarted = false;

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
	
	UAbilityTask_WaitDelay* DistanceCheckTask = UAbilityTask_WaitDelay::WaitDelay(this, CheckInterval);
	if (!DistanceCheckTask)
	{
		K2_EndAbility();
		return;
	}

	DistanceCheckTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnDistanceCheckTick);
	DistanceCheckTask->ReadyForActivation();
}

void UGA_SuicideAttack::OnDistanceCheckTick()
{
	if (bHasTriggeredExplosion)
	{
		return;
	}

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

	if (AICon)
	{
		if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
		{
			Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
		}
	}
	
	if (!Target)
	{
		if (!bHasTriggeredExplosion)
		{
			UAbilityTask_WaitDelay* LoopTask = UAbilityTask_WaitDelay::WaitDelay(this, CheckInterval);
			if (LoopTask)
			{
				LoopTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnDistanceCheckTick);
				LoopTask->ReadyForActivation();
			}
		}
		return;
	}
	
	FVector ML = Monster->GetActorLocation();
	FVector TL = Target->GetActorLocation();
	ML.Z = TL.Z = 0;

	const float RawDist = FVector::Dist(ML, TL);

	float CapsuleOffset = 0.f;
	if (Monster->GetCapsuleComponent() && Target->GetRootComponent())
	{
		if (UCapsuleComponent* MCap = Monster->GetCapsuleComponent())
		{
			if (UCapsuleComponent* TCap = Cast<UCapsuleComponent>(Target->GetRootComponent()))
			{
				CapsuleOffset = MCap->GetScaledCapsuleRadius() + TCap->GetScaledCapsuleRadius();
			}
		}
	}
	
	const float VisualDist = RawDist - CapsuleOffset;
	
	if (VisualDist <= TriggerRange)
	{
		UAnimInstance* Anim = Monster->GetMesh() ? Monster->GetMesh()->GetAnimInstance() : nullptr;
		if (!Anim)
		{
			K2_EndAbility();
			return;
		}

		bHasTriggeredExplosion = true;
		
		UAbilityTask_PlayMontageAndWait* PlayTask =	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SuicideMontage);

		if (!PlayTask)
		{
			K2_EndAbility();
			return;
		}

		PlayTask->OnCompleted.AddDynamic(this, &UGA_SuicideAttack::OnMontageCompleted);
		PlayTask->OnBlendOut.AddDynamic(this, &UGA_SuicideAttack::OnMontageCompleted);
		PlayTask->OnCancelled.AddDynamic(this, &UGA_SuicideAttack::OnMontageCancelled);
		PlayTask->OnInterrupted.AddDynamic(this, &UGA_SuicideAttack::OnMontageCancelled);

		PlayTask->ReadyForActivation();
		
		UAbilityTask_WaitGameplayEvent* DamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Damage")));

		if (DamageEvent)
		{
			DamageEvent->EventReceived.AddDynamic(this, &UGA_SuicideAttack::OnDamageEvent);
			DamageEvent->ReadyForActivation();
		}

		return;
	}
	
	if (!bHasTriggeredExplosion)
	{
		UAbilityTask_WaitDelay* LoopTask = UAbilityTask_WaitDelay::WaitDelay(this, CheckInterval);
		if (LoopTask)
		{
			LoopTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnDistanceCheckTick);
			LoopTask->ReadyForActivation();
		}
	}
}

void UGA_SuicideAttack::OnDamageEvent(FGameplayEventData Data)
{
	TArray<FHitResult> Hits = GetHitResultFromVirtualSocketTargetData(Data.TargetData);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
			continue;

		if (IgnoreTargets.Contains(HitActor))
			continue;

		if (DamageEffect)
			ApplyGameplayEffectToHitResultActor(Hit, DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		
		IgnoreTargets.Add(HitActor);
	}
}

void UGA_SuicideAttack::OnMontageCompleted()
{
	if (bKillDelayStarted)
	{
		return;
	}

	bKillDelayStarted = true;

	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, KillDelay);
	if (!DelayTask)
	{
		OnKillDelayFinished();
		return;
	}

	DelayTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnKillDelayFinished);
	DelayTask->ReadyForActivation();
}

void UGA_SuicideAttack::OnMontageCancelled()
{
	if (bKillDelayStarted)
	{
		return;
	}

	bKillDelayStarted = true;

	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, KillDelay);
	if (!DelayTask)
	{
		OnKillDelayFinished();
		return;
	}

	DelayTask->OnFinish.AddDynamic(this, &UGA_SuicideAttack::OnKillDelayFinished);
	DelayTask->ReadyForActivation();
}

void UGA_SuicideAttack::OnKillDelayFinished()
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid())
	{
		K2_EndAbility();
		return;
	}

	if (AMonster* Monster = Cast<AMonster>(CurrentActorInfo->AvatarActor.Get()))
	{
		Monster->Deactivate();
	}

	K2_EndAbility();
}
