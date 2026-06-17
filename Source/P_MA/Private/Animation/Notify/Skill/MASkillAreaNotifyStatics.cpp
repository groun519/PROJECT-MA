#include "Animation/Notify/Skill/MASkillAreaNotifyStatics.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Area/MASkillAreaTargetData.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"

bool MASkillAreaNotifyStatics::ResolveOriginTransform(USkeletalMeshComponent* MeshComp, FTransform& OutOriginTransform)
{
	if (!MeshComp) return false;

	if (MeshComp->GetNumBones() > 0)
	{
		const FName RootBoneName = MeshComp->GetBoneName(0);
		OutOriginTransform = FTransform(
			MeshComp->GetBoneQuaternion(RootBoneName),
			MeshComp->GetBoneLocation(RootBoneName),
			FVector::OneVector);
	}
	else
	{
		OutOriginTransform = MeshComp->GetComponentTransform();
	}
	return true;
}

void MASkillAreaNotifyStatics::AppendTargetData(FMASkillEvent& OutEvent, const FMASkillWorldAreaShape& Area)
{
	FGameplayAbilityTargetDataHandle TargetData;

	if (Area.Shape != EMASkillAreaShape::None)
	{
		auto* AreaTargetData = new FGameplayAbilityTargetData_SkillArea();
		AreaTargetData->Area = Area;
		TargetData.Add(AreaTargetData);
	}

	OutEvent.SetTargetData(TargetData);
}

void MASkillAreaNotifyStatics::DrawEditorPreview(
	UWorld* World,
	const FMASkillWorldAreaShape& Area)
{
#if WITH_EDITOR
	if (!IsEditorPreviewWorldNoPIE(World)) return;

	FlushPersistentDebugLines(World);
	MASkillAreaStatics::DrawWorldPreview(*World, Area);
#endif
}

bool MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(const UWorld* World)
{
#if WITH_EDITOR
	if (!World) return false;
	return World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview;
#else
	return false;
#endif
}
