// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_ChargeFwd.h"

#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Character/MACharacter.h"


void USkillBehavior_ChargeFwd::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!Character || !TargetActorClass || !RangeActorClass)
		return;
	
	TargetActor = GetWorld()->SpawnActor<AMATargetActor_ImedDamageFwd>(TargetActorClass);
	if (TargetActor)
	{
		TargetActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ChargeValueChanged.AddUObject(TargetActor, &AMATargetActor_ImedDamageFwd::HandleChargeValueChanged);
	}
	
	//스킬 사정거리 액터 붙이고 크기 설정
	MaxRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(RangeActorClass);
	CurrentRangeActor= GetWorld()->SpawnActor<AMAAbilityRangeActor>(RangeActorClass);
	if (MaxRangeActor && CurrentRangeActor)
	{
		MaxRangeActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		MaxRangeActor->SetAbilityRange(MaxTraceDistance);
		CurrentRangeActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		CurrentRangeActor->SetAbilityRange(MinTraceDistance);
	}

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	InputReleaseTask->OnRelease.AddDynamic(this, &USkillBehavior_ChargeFwd::OnKeyReleased);
	InputReleaseTask->ReadyForActivation();

	SkillTimeoutTask= UAbilityTask_WaitDelay::WaitDelay(OwningAbility, SkillTimeoutDuration);
	SkillTimeoutTask->OnFinish.AddDynamic(this, &USkillBehavior_ChargeFwd::OnSkillTimeout);
	SkillTimeoutTask->ReadyForActivation();

	ChargeStartTime = GetWorld()->GetTimeSeconds();
	GetWorld()->GetTimerManager().SetTimer(ChargeUpdateHandle,this ,&USkillBehavior_ChargeFwd::ChargeUpdate, 0.05f,true);
}

void USkillBehavior_ChargeFwd::OnEndAbility_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateHandle);
	
	if (InputReleaseTask.IsValid())
		InputReleaseTask->EndTask();
	if (SkillTimeoutTask.IsValid())
		SkillTimeoutTask->EndTask();
	if (TargetActor)
	{
		TargetActor->Destroy();
		TargetActor=nullptr;
	}
	if (MaxRangeActor)
	{
		MaxRangeActor->Destroy();
		MaxRangeActor=nullptr;
	}
	if (CurrentRangeActor)
	{
		CurrentRangeActor->Destroy();
		CurrentRangeActor=nullptr;
	}
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_ChargeFwd::OnKeyReleased(float TimeHeld)
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateHandle);
	if (!TargetActor)
		return;
	OwningAbility->RequestEndAbility();
	
}

void USkillBehavior_ChargeFwd::OnSkillTimeout()
{
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_ChargeFwd::ChargeUpdate()
{
	float ElapsedTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	float CurrentChargeRatio = FMath::Clamp(ElapsedTime / MaxChargeDuration , 0.f, 10.f);
	//UE_LOG(LogTemp, Warning, TEXT("CurrentChargeRatio = %f"), CurrentChargeRatio);
	ChargeValueChanged.Broadcast(CurrentChargeRatio);
}
