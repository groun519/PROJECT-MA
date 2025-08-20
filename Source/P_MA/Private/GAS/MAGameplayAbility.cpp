// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"

class UAnimInstance* UMAGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
	{
		return OwnerSkeletalMeshComp->GetAnimInstance();
	}
	return nullptr;
}