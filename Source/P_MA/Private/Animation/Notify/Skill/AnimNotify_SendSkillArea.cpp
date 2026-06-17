#include "Animation/Notify/Skill/AnimNotify_SendSkillArea.h"

#include "Animation/Notify/Skill/MASkillAreaNotifyStatics.h"
#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"

void UAnimNotify_SendSkillArea::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UWorld* World = MeshComp ? MeshComp->GetWorld() : nullptr;
	if (!World) return;

	FTransform OriginTransform;
	if (!MASkillAreaNotifyStatics::ResolveOriginTransform(MeshComp, OriginTransform)) return;

	UMASkillAbility* SkillAbility = MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	const FMASkillWorldAreaShape WorldArea = Area.ResolveWorld(
		OriginTransform,
		MASkillAnimNotifyStatics::ResolveSkillAreaScale(SkillAbility));
	if (!WorldArea.IsValid()) return;

	MASkillAreaNotifyStatics::DrawEditorPreview(World, WorldArea);
	if (World->IsPreviewWorld() || MASkillAreaNotifyStatics::IsEditorPreviewWorldNoPIE(World)) return;
	if (!EventTag.IsValid()) return;

	if (!SkillAbility) return;

	FMASkillEvent Event(EventTag);
	MASkillAreaNotifyStatics::AppendTargetData(Event, WorldArea);
	UMASkillEventRoutingStatics::TryNotifySkillEvent(SkillAbility, MoveTemp(Event), SkillAbility->GetCurrentBindingScope());
}
