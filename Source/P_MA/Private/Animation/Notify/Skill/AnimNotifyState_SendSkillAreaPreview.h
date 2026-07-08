#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_SendSkillAreaPreview.generated.h"

class UDecalComponent;

struct FMASkillActiveAreaPreview
{
	TWeakObjectPtr<UDecalComponent> Decal;
	float AreaScale = 1.f;
	FGameplayTag VisualTag;
	bool bContextReady = false;
};

UCLASS()
class P_MA_API UAnimNotifyState_SendSkillAreaPreview : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	UPROPERTY(EditAnywhere, Category="Gameplay Ability")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category="Preview")
	bool bAttachPreviewToMesh = false;

	UPROPERTY(EditAnywhere, Category="Area", meta=(ShowOnlyInnerProperties))
	FMASkillAreaShape Area;

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FMASkillActiveAreaPreview> ActivePreviews;

	bool ResolveWorldArea(USkeletalMeshComponent* MeshComp, float AreaScale, FMASkillWorldAreaShape& OutArea) const;
	bool ResolvePreviewContext(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation, FMASkillActiveAreaPreview& OutPreview) const;
	void DestroyPreviewDecal(USkeletalMeshComponent* MeshComp);
	void SpawnPreviewDecal(USkeletalMeshComponent* MeshComp, FGameplayTag VisualTag, const FMASkillWorldAreaShape& WorldArea);
	void UpdatePreviewDecalTransform(USkeletalMeshComponent* MeshComp, const FMASkillWorldAreaShape& WorldArea);
};
