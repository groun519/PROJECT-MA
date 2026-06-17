#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_SendSkillArea.generated.h"

UCLASS()
class UAnimNotify_SendSkillArea : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category="Gameplay Ability")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category="Area", meta=(ShowOnlyInnerProperties))
	FMASkillAreaShape Area;
};
