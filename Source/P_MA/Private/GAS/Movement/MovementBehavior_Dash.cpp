// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MovementBehavior_Dash.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/MACharacter.h"
#include "Components/CapsuleComponent.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Ability/SkillBehaviorConfig.h"

void UMovementBehavior_Dash::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	
	WaitDashStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DashStartTag);
	WaitDashStartEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Dash::OnDashStartEventReceived);
	WaitDashStartEventTask->ReadyForActivation();


}

void UMovementBehavior_Dash::OnEndAbility_Implementation()
{
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	
	if (WaitDashStartEventTask.IsValid())
		WaitDashStartEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Dash::InitFromConfig(const FInstancedStruct& ConfigPayload)
{
	Super::InitFromConfig(ConfigPayload);
	const FConfig_Dash* DashConfig = ConfigPayload.GetPtr<FConfig_Dash>();
	if (DashConfig)
	{
		MontageToPlay=DashConfig->MontageToPlay;
		UpLaunchForce = DashConfig->UpLaunchForce;
		ForwardLaunchForce = DashConfig->ForwardLaunchForce;
		VFXDataSet=DashConfig->VFXDataSet;
	}
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
