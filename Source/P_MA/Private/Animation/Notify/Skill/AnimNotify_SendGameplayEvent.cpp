// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notify/Skill/AnimNotify_SendGameplayEvent.h"
#include "Animation/MAAnimInstance.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GameplayTagsManager.h"

namespace
{
	UMASkillAbility* ResolveGameplayEventOwnerSkillAbility(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation)
	{
		if (!MeshComp || !Animation) return nullptr;

		const UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(MeshComp->GetAnimInstance());
		return AnimInstance ? AnimInstance->FindAnimationOwner(Animation) : nullptr;
	}
}

void UAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World || World->IsPreviewWorld()) return;

	if (UMASkillAbility* SkillAbility = ResolveGameplayEventOwnerSkillAbility(MeshComp, Animation))
	{
		FGameplayEventData Data;
		Data.EventTag = MontageEventTag;
		SkillAbility->SendSkillGameplayEvent(Data, SkillAbility->GetCurrentBindingScope());
	}
}

FString UAnimNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (MontageEventTag.IsValid())
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(MontageEventTag, TagNames);
		return TagNames.Last().ToString();
	}
	return "None";
}


