#include "Animation/Notify/Skill/AnimNotify_SendGameplayEvent.h"

#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GameplayTagsManager.h"

void UAnimNotify_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                             const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World || World->IsPreviewWorld()) return;

	if (UMASkillAbility* SkillAbility = MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation))
	{
		UMASkillEventRoutingStatics::TryNotifySkillEvent(
			SkillAbility,
			FMASkillEvent(MontageEventTag),
			SkillAbility->GetCurrentBindingScope());
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


