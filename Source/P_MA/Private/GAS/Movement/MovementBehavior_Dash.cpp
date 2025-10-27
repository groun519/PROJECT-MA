// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MovementBehavior_Dash.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimNotify_SendNewPlayerTrans.h"
#include "Character/MACharacter.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

void UMovementBehavior_Dash::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	WaitDashStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DashStartTag);
	WaitDashStartEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Dash::OnDashStartEventReceived);
	WaitDashStartEventTask->ReadyForActivation();

	WaitDamageTagEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
	WaitDamageTagEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Dash::OnDamageEventReceived);
	WaitDamageTagEventTask->ReadyForActivation();
}

void UMovementBehavior_Dash::OnEndAbility_Implementation()
{
	if (WaitDashStartEventTask.IsValid())
		WaitDamageTagEventTask->EndTask();
	if (WaitDamageTagEventTask.IsValid())
		WaitDamageTagEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Dash::OnDashStartEventReceived(FGameplayEventData Payload)
{
	if (Payload.TargetData.Num() >0)
	{
		const FGameplayAbilityTargetData* TargetData = Payload.TargetData.Get(0);
		if (TargetData && TargetData->GetScriptStruct()->IsChildOf(FDashData::StaticStruct()))
		{
			const FDashData* DashData = static_cast<const FDashData*>(TargetData);

			const float ReceivedDashForce = DashData->DashForce;

			if (Character)
			{
				FVector LaunchVelocity = Character->GetActorForwardVector() * ReceivedDashForce;
				LaunchVelocity.Z += UpLaunchForce;
				Character->LaunchCharacter(LaunchVelocity, true,true);
			}
		}
	}
}

void UMovementBehavior_Dash::OnDamageEventReceived(FGameplayEventData Payload)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
	for (FHitResult& HitResult : HitResults)
	{
		OwningAbility->ApplyGameplayEffectToHitResultActor(HitResult, MovementDamageEffect, OwningAbility->GetAbilityLevel());
	}
}
