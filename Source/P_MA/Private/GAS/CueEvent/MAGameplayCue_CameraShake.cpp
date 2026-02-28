// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/CueEvent/MAGameplayCue_CameraShake.h"

#include "GAS/MAGameplayAbilityTypes.h"

bool UMAGameplayCue_CameraShake::OnExecute_Implementation(AActor* MyTarget,
                                                          const FGameplayCueParameters& Parameters) const
{
	bool bIsCritical = false;
	if (const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(Parameters.EffectContext.Get()))
	{
		bIsCritical = MAContext->IsCriticalHit();
	}
	UE_LOG(LogTemp,Warning, TEXT("bIsCritical == %d"), bIsCritical);
	
	TSubclassOf<UCameraShakeBase> ShakeClass = bIsCritical ? CriticalCameraShake : RegularCameraShake;
	UE_LOG(LogTemp,Warning, TEXT("%s"), *ShakeClass->GetName());
	if (ShakeClass)
	{
		AActor* Attacker = Parameters.Instigator.Get();
		if (Attacker)
		{
			if (APawn* AttackerPawn = Cast<APawn>(Attacker))
			{
				if (APlayerController* PC = Cast<APlayerController>(AttackerPawn->GetController()))
				{
					PC->ClientStartCameraShake(ShakeClass);
				}
			}
		}
	}
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
