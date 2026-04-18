// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notify/AnimNotifyState_SendComboStartEnd.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAnimNotifyState_SendComboStartEnd::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!MeshComp->GetOwner()) return;
	// ASC가 없다면 리턴
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner())) return;

	// BeginEventTag 이벤트 전송 (ex. Combo01)
	if (bUseBeginEvent && BeginEventTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), BeginEventTag, FGameplayEventData());
}

void UAnimNotifyState_SendComboStartEnd::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp->GetOwner()) return;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), ClearEventTag, FGameplayEventData());
	
	if (bUseEndEvent && EndEventTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EndEventTag, FGameplayEventData());
}
