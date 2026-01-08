// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Ability/GA_GiantSwing.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/MACharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MAAbilitySystemStatics.h"

void UGA_GiantSwing::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	IgnoreTargets.Empty();

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	ACharacter* Monster = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Monster || !GiantSwingMontage)
	{
		K2_EndAbility();
		return;
	}

	UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance();
	if (!Anim)
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, GiantSwingMontage);

	PlayMontageTask->OnBlendOut.AddDynamic(this, &UGA_GiantSwing::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_GiantSwing::K2_EndAbility);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_GiantSwing::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_GiantSwing::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitDamageEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Damage")));
	WaitDamageEvent->EventReceived.AddDynamic(this, &UGA_GiantSwing::OnDamageEvent);
	WaitDamageEvent->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitEndEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Monster.Ability.End")));
	WaitEndEvent->EventReceived.AddDynamic(this, &UGA_GiantSwing::OnEndEventReceived);
	WaitEndEvent->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitGrabEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Monster.Ability.GiantSwing.Grab")));
	WaitGrabEvent->EventReceived.AddDynamic(this, &UGA_GiantSwing::OnGrabEvent);
	WaitGrabEvent->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitSwingEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Monster.Ability.GiantSwing.Swing")));
	WaitSwingEvent->EventReceived.AddDynamic(this, &UGA_GiantSwing::OnSwingEvent);
	WaitSwingEvent->ReadyForActivation();
}
void UGA_GiantSwing::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ACharacter* Monster = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	AMACharacter* TargetCharacter = Cast<AMACharacter>(GrabbedTarget);

	if (GrabbedTarget)
	{
		GrabbedTarget->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GrabbedTarget = nullptr;
	}

	if (TargetCharacter)
	{
		if (APlayerController* PC = Cast<APlayerController>(TargetCharacter->GetController()))
		{
			TargetCharacter->EnableInput(PC);
		}
	}

	if (Monster)
	{
		if (UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance())
		{
			if (GiantSwingMontage && Anim->Montage_IsPlaying(GiantSwingMontage))
			{
				Anim->Montage_Stop(0.1f, GiantSwingMontage);
			}
		}

		if (UCharacterMovementComponent* Move = Monster->GetCharacterMovement())
		{
			Move->SetMovementMode(MOVE_Walking);
		}

		if (AAIController* AI = Cast<AAIController>(Monster->GetController()))
		{
			if (AI->BrainComponent)
			{
				AI->BrainComponent->RestartLogic();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_GiantSwing::OnDamageEvent(FGameplayEventData Data)
{
	IgnoreTargets.Empty();

	TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(Data.TargetData);
	for (const FHitResult& Hit : HitResults)
	{
		if (IgnoreTargets.Contains(Hit.GetActor()))
			continue;

		ApplyGameplayEffectToHitResultActor(Hit, DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));

		IgnoreTargets.Add(Hit.GetActor());
	}
}

void UGA_GiantSwing::OnEndEventReceived(FGameplayEventData Data)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_GiantSwing::OnGrabEvent(FGameplayEventData Data)
{
	ACharacter* Monster = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Monster) return;

	USkeletalMeshComponent* Mesh = Monster->GetMesh();
	if (!Mesh || !Mesh->DoesSocketExist(MonsterGrabSocketName)) return;

	TArray<FHitResult> Hits = GetHitResultFromVirtualSocketTargetData(Data.TargetData);
	if (Hits.IsEmpty()) return;

	ACharacter* Target = Cast<ACharacter>(Hits[0].GetActor());
	if (!Target) return;

	GrabbedTarget = Target;

	if (APlayerController* PC = Cast<APlayerController>(Target->GetController()))
	{
		Target->DisableInput(PC);
	}

	Target->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MonsterGrabSocketName);

	if (UAnimInstance* Anim = Monster->GetMesh()->GetAnimInstance())
	{
		Anim->Montage_JumpToSection(TEXT("Swing"), GiantSwingMontage);
	}
}

void UGA_GiantSwing::OnSwingEvent(FGameplayEventData Data)
{
	if (GrabbedTarget)
	{
		GrabbedTarget->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		ACharacter* Monster = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (!Monster)
			return;
        
		FRotator MonsterRotation = Monster->GetActorRotation();
		FVector ForwardDirection = MonsterRotation.Vector();
		FVector LeftDirection = MonsterRotation.Quaternion().GetAxisX();

		FVector Impulse = -(ForwardDirection * 0.5f + LeftDirection * 0.5f) * 750.f;

		FRotator AngleOffset(0.f, 15.f, 0.f);
		Impulse = AngleOffset.RotateVector(Impulse);
		Impulse.Z += 700.f;
        
		if (ACharacter* Character = Cast<ACharacter>(GrabbedTarget))
		{
			Character->LaunchCharacter(Impulse, true, true);

			FGameplayEventData EventData;
			EventData.Instigator = GetAvatarActorFromActorInfo();
			EventData.Target = Character;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				Character,
				FGameplayTag::RequestGameplayTag(TEXT("Stats.Knockdown")),
				EventData
			);
		}
	}
}