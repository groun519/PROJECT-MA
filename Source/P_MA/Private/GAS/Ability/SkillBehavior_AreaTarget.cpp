// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_AreaTarget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/MACharacter.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/MATargetActor.h"
#include "GAS/MAAbilityRangeActor.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

void USkillBehavior_AreaTarget::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	if (!OwningAbility || !Character)
		return;

	if (RangeActorClass)
	{
		SpawnedRangeActor = GetWorld()->SpawnActor<AMAAbilityRangeActor>(RangeActorClass);
		if (SpawnedRangeActor)
		{
			SpawnedRangeActor->AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			SpawnedRangeActor->SetAbilityRange(MaxRange);
		}
	}
	
	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask -> ValidData.AddDynamic(this, &USkillBehavior_AreaTarget::TargetConfirmed);
	WaitTargetDataTask -> Cancelled.AddDynamic(this, &USkillBehavior_AreaTarget::TargetCancelled);
	WaitTargetDataTask -> ReadyForActivation();
	
	// 미리보기 상태로 스폰
	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask -> BeginSpawningActor(OwningAbility, TargetActorClass, TargetActor);
	AMATargetActor* GroundPick = Cast<AMATargetActor>(TargetActor);
	if (GroundPick)
	{
		GroundPick -> SetTargetAreaRadius(AbilitySize);
		GroundPick -> SetTargetTraceRange(MaxRange);
	}
	// 미리보기 최종 결정
	WaitTargetDataTask -> FinishSpawningActor(OwningAbility, TargetActor);
}

void USkillBehavior_AreaTarget::OnEndAbility_Implementation()
{
	if (SpawnedRangeActor)
		SpawnedRangeActor->Destroy();
	SpawnedRangeActor = nullptr;
	
	Super::OnEndAbility_Implementation();
}

void USkillBehavior_AreaTarget::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	OwningAbility->RequestEndAbility();
}

void USkillBehavior_AreaTarget::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	OwningAbility->RequestEndAbility();
}
