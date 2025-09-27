// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/GameplayAbility_Teleport.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/Character.h"
#include "Player/MAPlayerCharacter.h"

UGameplayAbility_Teleport::UGameplayAbility_Teleport()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGameplayAbility_Teleport::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,SkillMontage);
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_Teleport::K2_EndAbility);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_Teleport::K2_EndAbility);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_Teleport::K2_EndAbility);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_Teleport::K2_EndAbility);
	PlayMontageTask->ReadyForActivation();

	
	if (IsLocallyControlled())
	{
		ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		APlayerController* PC = Character ? Cast<APlayerController>(Character->GetController()) : nullptr;
		if (!Character || !PC)
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
			return;
		}
		
		FVector TeleportTargetLocation;
		bool bFoundLocation = false;

		FVector WorldOrigin, WorldDir;
		if (PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDir))
		{
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Character);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, WorldOrigin + WorldDir * 15000.f, ECC_Visibility, Params))
			{
				const FVector MouseWorldLocation = HitResult.Location;
				const FVector CharacterLocation = Character->GetActorLocation();
				TeleportTargetLocation = MouseWorldLocation;

				const float DistanceToMouse = FVector::Dist(CharacterLocation, MouseWorldLocation);
				if (DistanceToMouse > MaxTeleportDistance)
				{
					const FVector Direction = (MouseWorldLocation - CharacterLocation).GetSafeNormal();
					TeleportTargetLocation = CharacterLocation + Direction * MaxTeleportDistance;
				}
			
				FVector StartTrace = FVector(TeleportTargetLocation.X, TeleportTargetLocation.Y, TeleportTargetLocation.Z + GroundTraceDistance);
				FVector EndTrace = FVector(TeleportTargetLocation.X, TeleportTargetLocation.Y, TeleportTargetLocation.Z - GroundTraceDistance);
				FHitResult GroundHit;
				if (GetWorld()->LineTraceSingleByChannel(GroundHit, StartTrace, EndTrace, ECC_Visibility))
				{
					TeleportTargetLocation = GroundHit.Location;
				}
				bFoundLocation = true;
			}
		}

		if (bFoundLocation)
		{
			Server_ExecuteTeleport(TeleportTargetLocation);
		}
		else
		{
			CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		}
	}
	/*
	UAbilityTask_WaitGameplayEvent* WaitStartTeleportTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Movement.Teleport.Start"));
	WaitStartTeleportTask->EventReceived.AddDynamic(this, &UGameplayAbility_Teleport::StartTeleporting);
	WaitStartTeleportTask->ReadyForActivation();
	*/
}

void UGameplayAbility_Teleport::StartTeleporting(FGameplayEventData Data)
{
	/*
	UE_LOG(LogTemp, Warning, TEXT("Start Teleporting"));
	FVector_NetQuantize TargetVector=FVector::ZeroVector;
	Server_ExecuteTeleport(TargetVector);
	*/
}

void UGameplayAbility_Teleport::Server_ExecuteTeleport_Implementation(FVector_NetQuantize TargetLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("[SERVER] RPC Received. Requesting Teleport from Character..."));

	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(GetAvatarActorFromActorInfo());
	if(Character)
	{
		// 캐릭터의 새로운 텔레포트 요청 함수를 호출합니다.
		Character->RequestTeleport(TargetLocation);
	}
}
