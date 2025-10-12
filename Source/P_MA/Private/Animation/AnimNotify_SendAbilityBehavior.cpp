// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_SendAbilityBehavior.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"


void UAnimNotify_SendAbilityBehavior::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
		if (Owner)
		{
			FGameplayEventData Data;
			Data.EventTag = MontageEventTag;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner,Data.EventTag,Data);
		}
	}
}
