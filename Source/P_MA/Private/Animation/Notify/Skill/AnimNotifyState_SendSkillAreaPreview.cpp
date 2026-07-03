#include "Animation/Notify/Skill/AnimNotifyState_SendSkillAreaPreview.h"

#include "Animation/Notify/Skill/MASkillAreaNotifyStatics.h"
#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"
#include "Components/DecalComponent.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"

bool UAnimNotifyState_SendSkillAreaPreview::ResolveWorldArea(
	USkeletalMeshComponent* MeshComp,
	const UAnimSequenceBase* Animation,
	FMASkillWorldAreaShape& OutArea) const
{
	FTransform OriginTransform;
	if (!MASkillAreaNotifyStatics::ResolveOriginTransform(MeshComp, OriginTransform)) return false;

	OutArea = Area.ResolveWorld(
		OriginTransform,
		MASkillAnimNotifyStatics::ResolveSkillAreaScale(MeshComp, Animation));
	return OutArea.IsValid();
}

void UAnimNotifyState_SendSkillAreaPreview::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UWorld* World = MeshComp ? MeshComp->GetWorld() : nullptr;
	if (!World) return;

	FMASkillWorldAreaShape WorldArea;
	if (!ResolveWorldArea(MeshComp, Animation, WorldArea)) return;

	MASkillAreaNotifyStatics::DrawEditorPreview(World, WorldArea);
	if (World->GetNetMode() == NM_DedicatedServer || MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(World)) return;

	SpawnPreviewDecal(MeshComp, Animation, WorldArea);
}

void UAnimNotifyState_SendSkillAreaPreview::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!MeshComp) return;

	DestroyPreviewDecal(MeshComp);

	UWorld* World = MeshComp->GetWorld();
	if (!World || MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(World)) return;
	if (!EventTag.IsValid()) return;

	FMASkillWorldAreaShape WorldArea;
	if (!ResolveWorldArea(MeshComp, Animation, WorldArea)) return;

	UMASkillAbility* SkillAbility = MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	if (!SkillAbility) return;

	FMASkillEvent Event(EventTag);
	MASkillAreaNotifyStatics::AppendTargetData(Event, WorldArea);
	UMASkillEventRoutingStatics::TryNotifySkillEvent(SkillAbility, MoveTemp(Event), SkillAbility->GetCurrentBindingScope());
}

void UAnimNotifyState_SendSkillAreaPreview::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	UWorld* World = MeshComp ? MeshComp->GetWorld() : nullptr;
	if (!World
		|| World->GetNetMode() == NM_DedicatedServer
		|| MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(World)
		|| !ActiveDecals.Contains(MeshComp))
	{
		return;
	}

	FMASkillWorldAreaShape WorldArea;
	if (ResolveWorldArea(MeshComp, Animation, WorldArea)) UpdatePreviewDecalTransform(MeshComp, WorldArea);
}

FString UAnimNotifyState_SendSkillAreaPreview::GetNotifyName_Implementation() const
{
	return TEXT("Send Skill Area Preview");
}

void UAnimNotifyState_SendSkillAreaPreview::DestroyPreviewDecal(USkeletalMeshComponent* MeshComp)
{
	TWeakObjectPtr<UDecalComponent>* FoundDecal = ActiveDecals.Find(MeshComp);
	if (!FoundDecal) return;

	if (UDecalComponent* DecalComponent = FoundDecal->Get())
	{
		DecalComponent->DestroyComponent();
	}
	ActiveDecals.Remove(MeshComp);
}

void UAnimNotifyState_SendSkillAreaPreview::SpawnPreviewDecal(
	USkeletalMeshComponent* MeshComp,
	const UAnimSequenceBase* Animation,
	const FMASkillWorldAreaShape& WorldArea)
{
	if (!MeshComp) return;

	DestroyPreviewDecal(MeshComp);
	const UMASkillAbility* SkillAbility = MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	UDecalComponent* DecalComponent = MASkillAreaDecalStatics::SpawnPreview(
		MeshComp->GetOwner(),
		bAttachPreviewToMesh ? MeshComp : nullptr,
		SkillAbility,
		WorldArea);
	if (DecalComponent) ActiveDecals.Add(MeshComp, DecalComponent);
}

void UAnimNotifyState_SendSkillAreaPreview::UpdatePreviewDecalTransform(
	USkeletalMeshComponent* MeshComp,
	const FMASkillWorldAreaShape& WorldArea)
{
	const TWeakObjectPtr<UDecalComponent>* FoundDecal = ActiveDecals.Find(MeshComp);
	UDecalComponent* DecalComponent = FoundDecal ? FoundDecal->Get() : nullptr;
	if (DecalComponent)
	{
		MASkillAreaDecalStatics::SetAreaTransform(*DecalComponent, WorldArea);
	}
}
