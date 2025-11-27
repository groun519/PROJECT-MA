// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MovementBehavior_Dash.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/MACharacter.h"
#include "Components/CapsuleComponent.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

void UMovementBehavior_Dash::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	WaitDashStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DashStartTag);
	WaitDashStartEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Dash::OnDashStartEventReceived);
	WaitDashStartEventTask->ReadyForActivation();

	if (OwningAbility->K2_HasAuthority())
	{
		WaitDamageTagEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
		WaitDamageTagEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Dash::OnDamageEventReceived);
		WaitDamageTagEventTask->ReadyForActivation();
	}
}

void UMovementBehavior_Dash::OnEndAbility_Implementation()
{
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	
	if (WaitDashStartEventTask.IsValid())
		WaitDashStartEventTask->EndTask();
	if (WaitDamageTagEventTask.IsValid())
		WaitDamageTagEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Dash::InitFromData(const FSkillDefinitionDT& Data)
{
	Super::InitFromData(Data);

	MontageToPlay=Data.DashData.MontageToPlay;
	VFXDataSet=Data.DashData.VFXDataSet;
	
	if (Data.DashData.DamageMultiplier>0.f)		BehaviorDamageMultiplier = Data.DashData.DamageMultiplier;
	if (Data.DashData.CooldownDuration>0.f)		CooldownDuration = Data.DashData.CooldownDuration;
	if (Data.DashData.ForwardLaunchForce>0.f)	ForwardLaunchForce = Data.DashData.ForwardLaunchForce;
	if (Data.DashData.UpLaunchForce>0.f)		UpLaunchForce = Data.DashData.UpLaunchForce;
}

void UMovementBehavior_Dash::OnDashStartEventReceived(FGameplayEventData Payload)
{
	if (Payload.TargetData.Num() >0)
	{
		const FGameplayAbilityTargetData* TargetData = Payload.TargetData.Get(0);
		if (TargetData)
		{
			if (Character)
			{
				FVector LaunchVelocity = Character->GetActorForwardVector() * ForwardLaunchForce;
				LaunchVelocity.Z += UpLaunchForce;
				Character->LaunchCharacter(LaunchVelocity, true,true);
			}
		}
	}
}

void UMovementBehavior_Dash::OnDamageEventReceived(FGameplayEventData Payload)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(Payload.TargetData);
	OwningAbility->ApplyDamageToHitResults(HitResults);
}
