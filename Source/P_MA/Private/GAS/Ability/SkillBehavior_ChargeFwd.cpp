// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_ChargeFwd.h"

#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/MACharacter.h"


void USkillBehavior_ChargeFwd::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!Character || !TargetActorClass)
		return;
	
	TargetActor = GetWorld()->SpawnActor<AMATargetActor_ImedDamageFwd>(TargetActorClass);
	if (TargetActor)
	{
		TargetActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		TargetActor->StartTargeting(OwningAbility);
		TargetActor->Initialize(MaxTraceDistance, MinTraceDistance, SkillWidth, DecalDepth, MaxChargeDuration);
	}
	
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	InputReleaseTask->OnRelease.AddDynamic(this, &USkillBehavior_ChargeFwd::OnKeyReleased);
	InputReleaseTask->ReadyForActivation();

	SkillTimeoutTask= UAbilityTask_WaitDelay::WaitDelay(OwningAbility, SkillTimeoutDuration);
	SkillTimeoutTask->OnFinish.AddDynamic(this, &USkillBehavior_ChargeFwd::OnSkillTimeout);
	SkillTimeoutTask->ReadyForActivation();
}

void USkillBehavior_ChargeFwd::OnEndAbility_Implementation()
{
	if (InputReleaseTask.IsValid())
		InputReleaseTask->EndTask();
	if (SkillTimeoutTask.IsValid())
		SkillTimeoutTask->EndTask();
	if (TargetActor)
	{
		TargetActor->Destroy();
		TargetActor=nullptr;
	}

	Super::OnEndAbility_Implementation();
}

void USkillBehavior_ChargeFwd::OnKeyReleased(float TimeHeld)
{
	if (!TargetActor || !OwningAbility)
		return;
	
	float ChargeRatio = FMath::Clamp(TimeHeld / MaxChargeDuration, 0.f, 1.f);
	float FinalLength = FMath::Lerp(MinTraceDistance, MaxTraceDistance, ChargeRatio);
	SpawnVFX(FinalLength);
	
	FGameplayAbilityTargetDataHandle TargetDataHandle = TargetActor->GetTargetData();
	if (OwningAbility->K2_HasAuthority())
		OwningAbility->ApplyDamageToTargetData(TargetDataHandle, DamageEffect);
	
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeFwd::OnSkillTimeout()
{
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeFwd::SpawnVFX(float FinalLength)
{
	if (!ExecutionVFX || !Character)
		return;
	FVector Location = Character->GetActorLocation();
	FRotator Rotation = Character->GetActorRotation();

	float SafeLength = (VFXLength == 0.f) ? 1.f : VFXLength;
	float SafeWidth = (VFXWidth == 0.f) ? 1.f : VFXWidth;

	FVector Scale = FVector (FinalLength / SafeLength, SkillWidth/SafeWidth, 1.f);
	FTransform SpawnTransform(Rotation,Location,Scale);
	Character->Multicast_PlayNiagara(ExecutionVFX,SpawnTransform);
}
