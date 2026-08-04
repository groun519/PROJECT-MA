#include "Animation/Notify/Skill/AnimNotifyState_SendSkillAreaPreview.h"

#include "Animation/MAAnimInstance.h"
#include "Animation/Notify/Skill/MASkillAreaNotifyStatics.h"
#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"
#include "Components/DecalComponent.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"
#include "GAS/Skill/MASkillManagerComponent.h"

bool UAnimNotifyState_SendSkillAreaPreview::ResolveWorldArea(
	USkeletalMeshComponent* MeshComp,
	float AreaScale,
	FMASkillWorldAreaShape& OutArea) const
{
	FTransform OriginTransform;
	if (!MASkillAreaNotifyStatics::ResolveOriginTransform(MeshComp, OriginTransform)) return false;

	OutArea = Area.ResolveWorld(OriginTransform, AreaScale);
	return OutArea.IsValid();
}

bool UAnimNotifyState_SendSkillAreaPreview::ResolvePreviewContext(
	USkeletalMeshComponent* MeshComp,
	const UAnimSequenceBase* Animation,
	FMASkillActiveAreaPreview& OutPreview) const
{
	if (UMASkillAbility* SkillAbility = MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation))
	{
		OutPreview.AreaScale = MASkillAnimNotifyStatics::ResolveSkillAreaScale(SkillAbility);
		OutPreview.VisualTag = SkillAbility->GetVisualElementTag();
		OutPreview.bContextReady = true;
		return true;
	}

	const UMAAnimInstance* AnimInstance = MeshComp ? Cast<UMAAnimInstance>(MeshComp->GetAnimInstance()) : nullptr;
	if (!AnimInstance || !AnimInstance->FindSkillAreaPreviewContext(
		Animation,
		OutPreview.AreaScale,
		OutPreview.VisualTag))
	{
		return false;
	}

	OutPreview.bContextReady = true;
	return true;
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

	FMASkillActiveAreaPreview ResolvedPreview;
	const bool bHasPreviewContext = ResolvePreviewContext(MeshComp, Animation, ResolvedPreview);
	FMASkillWorldAreaShape WorldArea;
	if (!ResolveWorldArea(MeshComp, ResolvedPreview.AreaScale, WorldArea)) return;

	MASkillAreaNotifyStatics::DrawEditorPreview(World, WorldArea);
	if (MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(World)) return;

	DestroyPreviewDecal(MeshComp);
	FMASkillActiveAreaPreview& ActivePreview = ActivePreviews.FindOrAdd(MeshComp);
	ActivePreview = ResolvedPreview;

	UMASkillAbility* SkillAbility = MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	if (SkillAbility && SkillAbility->K2_HasAuthority())
	{
		if (UMASkillManagerComponent* SkillManager = SkillAbility->GetSkillManagerComponent())
		{
			SkillManager->Multicast_RegisterSkillAreaPreviewContext(
				Animation,
				ResolvedPreview.AreaScale,
				ResolvedPreview.VisualTag);
		}
	}

	if (World->GetNetMode() == NM_DedicatedServer) return;
	if (bHasPreviewContext)
	{
		SpawnPreviewDecal(MeshComp, ResolvedPreview.VisualTag, WorldArea);
	}
}

void UAnimNotifyState_SendSkillAreaPreview::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!MeshComp) return;

	const FMASkillActiveAreaPreview* ActivePreview = ActivePreviews.Find(MeshComp);
	const bool bHasCapturedAreaScale = ActivePreview && ActivePreview->bContextReady;
	const float CapturedAreaScale = bHasCapturedAreaScale ? ActivePreview->AreaScale : 1.f;
	DestroyPreviewDecal(MeshComp);

	UWorld* World = MeshComp->GetWorld();
	if (!World || MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(World)) return;

	UMASkillAbility* SkillAbility = MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	if (SkillAbility && SkillAbility->K2_HasAuthority())
	{
		if (UMASkillManagerComponent* SkillManager = SkillAbility->GetSkillManagerComponent())
		{
			SkillManager->Multicast_UnregisterSkillAreaPreviewContext(Animation);
		}
	}

	if (!EventTag.IsValid()) return;
	if (!SkillAbility) return;

	const float AreaScale = bHasCapturedAreaScale
		? CapturedAreaScale
		: MASkillAnimNotifyStatics::ResolveSkillAreaScale(SkillAbility);
	FMASkillWorldAreaShape WorldArea;
	if (!ResolveWorldArea(MeshComp, AreaScale, WorldArea)) return;

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
		|| MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(World))
	{
		return;
	}

	FMASkillActiveAreaPreview* ActivePreview = ActivePreviews.Find(MeshComp);
	if (!ActivePreview) return;

	if (!ActivePreview->bContextReady && !ResolvePreviewContext(MeshComp, Animation, *ActivePreview)) return;

	FMASkillWorldAreaShape WorldArea;
	if (ResolveWorldArea(MeshComp, ActivePreview->AreaScale, WorldArea))
	{
		if (ActivePreview->Decal.IsValid())
		{
			UpdatePreviewDecalTransform(MeshComp, WorldArea);
		}
		else
		{
			SpawnPreviewDecal(MeshComp, ActivePreview->VisualTag, WorldArea);
		}
	}
}

FString UAnimNotifyState_SendSkillAreaPreview::GetNotifyName_Implementation() const
{
	return TEXT("Send Skill Area Preview");
}

void UAnimNotifyState_SendSkillAreaPreview::DestroyPreviewDecal(USkeletalMeshComponent* MeshComp)
{
	FMASkillActiveAreaPreview* ActivePreview = ActivePreviews.Find(MeshComp);
	if (!ActivePreview) return;

	if (UDecalComponent* DecalComponent = ActivePreview->Decal.Get())
	{
		DecalComponent->DestroyComponent();
	}
	ActivePreviews.Remove(MeshComp);
}

void UAnimNotifyState_SendSkillAreaPreview::SpawnPreviewDecal(
	USkeletalMeshComponent* MeshComp,
	FGameplayTag VisualTag,
	const FMASkillWorldAreaShape& WorldArea)
{
	FMASkillActiveAreaPreview& ActivePreview = ActivePreviews.FindOrAdd(MeshComp);
	if (UDecalComponent* ExistingDecal = ActivePreview.Decal.Get())
	{
		ExistingDecal->DestroyComponent();
	}

	UDecalComponent* DecalComponent = MASkillAreaDecalStatics::SpawnPreview(
		MeshComp->GetOwner(),
		bAttachPreviewToMesh ? MeshComp : nullptr,
		VisualTag,
		WorldArea);
	ActivePreview.Decal = DecalComponent;
}

void UAnimNotifyState_SendSkillAreaPreview::UpdatePreviewDecalTransform(
	USkeletalMeshComponent* MeshComp,
	const FMASkillWorldAreaShape& WorldArea)
{
	const FMASkillActiveAreaPreview* ActivePreview = ActivePreviews.Find(MeshComp);
	UDecalComponent* DecalComponent = ActivePreview ? ActivePreview->Decal.Get() : nullptr;
	if (DecalComponent)
	{
		MASkillAreaDecalStatics::SetAreaTransform(*DecalComponent, WorldArea);
	}
}
