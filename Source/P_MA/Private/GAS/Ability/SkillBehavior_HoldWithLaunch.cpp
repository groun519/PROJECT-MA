// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_HoldWithLaunch.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

void USkillBehavior_HoldWithLaunch::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	bHasLaunched = false;
	bHasSmashed = false;
	
	if (OwningAbility->K2_HasAuthority())
	{
		WaitLaunchTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility,LaunchTag);
		WaitLaunchTask->EventReceived.AddDynamic(this, &USkillBehavior_HoldWithLaunch::StartLaunch);
		WaitLaunchTask->ReadyForActivation();
	}

	WaitSmashTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility,SmashTag);
	WaitSmashTask->EventReceived.AddDynamic(this, &USkillBehavior_HoldWithLaunch::StartSmash);
	WaitSmashTask->ReadyForActivation();
}


void USkillBehavior_HoldWithLaunch::OnEndAbility_Implementation()
{
	if (WaitLaunchTask.IsValid())
		WaitLaunchTask->EndTask();
	if (WaitSmashTask.IsValid())
		WaitSmashTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}


void USkillBehavior_HoldWithLaunch::StartLaunch(FGameplayEventData Payload)
{
	if (OwningAbility->K2_HasAuthority())
	{
		float FinalSpeed = 0.f;
		if (!bHasLaunched)
		{
			FinalSpeed = FirstLaunchSpeed;
			bHasLaunched = true;
		}else
		{
			FinalSpeed = OtherLaunchSpeed;
		}
		if (bHasSmashed)
		{
			FinalSpeed = 0.f;
		}
		OwningAbility->PushTarget(OwningAbility->GetAvatarActorFromActorInfo(), FVector::UpVector * FinalSpeed);
		TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				OwningAbility->PushTarget(HitActor, FVector::UpVector * FinalSpeed*1.3f);
			}
		}
	}
}

void USkillBehavior_HoldWithLaunch::StartSmash(FGameplayEventData Payload)
{
	if (OwningAbility->K2_HasAuthority())
	{
		if (!bHasSmashed)
		{
			bHasSmashed = true;
			OwningAbility->PushTarget(OwningAbility->GetAvatarActorFromActorInfo(), FVector::DownVector * SmashSpeed);
			TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
			for (const FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				if (HitActor)
				{
					OwningAbility->PushTarget(HitActor, FVector::DownVector * SmashSpeed);
				}
			}
		}
	}
}

