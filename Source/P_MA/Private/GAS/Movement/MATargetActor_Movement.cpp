// Fill out your copyright notice in the Description page of Project Settings.

#include "Abilities/GameplayAbility.h"
#include "GAS/Movement/MATargetActor_Movement.h"

void AMATargetActor_Movement::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	PlayerController = Cast<APlayerController>(Ability ? Ability->GetCurrentActorInfo()->PlayerController : nullptr);
}

void AMATargetActor_Movement::ConfirmTargetingAndContinue()
{
	FHitResult Hit;
	if (PlayerController && PlayerController->GetHitResultUnderCursor(ECC_Visibility,true,Hit))
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;
		FGameplayAbilityTargetData_SingleTargetHit* NewData = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
		TargetDataHandle.Add(NewData);
		TargetDataReadyDelegate.Broadcast(TargetDataHandle);
	}
}
