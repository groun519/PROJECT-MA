#include "Animation/Notify/AnimNotify_SpawnNiagara.h"

#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/MACharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Weapon/WeaponComponent.h"

static USceneComponent* ResolveAttachComponent(USkeletalMeshComponent* MeshComp, const EMANiagaraAttachTarget AttachTarget)
{
	if (!MeshComp) return nullptr;
	if (AttachTarget != EMANiagaraAttachTarget::WeaponMesh) return MeshComp;

	if (const AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(MeshComp->GetOwner()))
	{
		if (UWeaponComponent* WeaponComponent = PlayerCharacter->GetWeaponComponent())
		{
			return WeaponComponent;
		}
	}

	return MeshComp;
}

static const UMASkillManagerComponent* ResolveSkillManager(const USkeletalMeshComponent* MeshComp)
{
	const AMACharacter* Character = MeshComp ? Cast<AMACharacter>(MeshComp->GetOwner()) : nullptr;
	return Character ? Character->GetSkillManagerComponent() : nullptr;
}

static FGameplayTag ResolveVisualElementTag(
	const UMASkillAbility* SkillAbility,
	const USkeletalMeshComponent* MeshComp)
{
	if (SkillAbility)
	{
		return SkillAbility->GetVisualElementTag();
	}

	const UMASkillManagerComponent* SkillManager = ResolveSkillManager(MeshComp);
	return SkillManager ? SkillManager->GetActivePreviewVisualElementTag() : FGameplayTag();
}

static bool ResolveVisualElementColor(FGameplayTag VisualElementTag, FLinearColor& OutColor)
{
	const FMAElementDataRow* ElementRow = FMAElementDataRow::FindByTag(
		VisualElementTag,
		TEXT("AnimNotify_SpawnNiagara"));
	if (!ElementRow) return false;

	OutColor = ElementRow->ElementColor;
	return true;
}

void UAnimNotify_SpawnNiagara::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp || !NiagaraTemplate) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	USceneComponent* AttachComponent = ResolveAttachComponent(MeshComp, AttachTarget);
	if (!AttachComponent) return;

	UMASkillAbility* SkillAbility = (bApplyElementalColor || bApplySkillAreaScale)
		? MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation)
		: nullptr;
	const FVector SpawnScale = bApplySkillAreaScale
		? Scale * MASkillAnimNotifyStatics::ResolveSkillAreaScale(SkillAbility)
		: Scale;
	FLinearColor VisualElementColor = FLinearColor::White;
	const bool bHasVisualElementColor = bApplyElementalColor
		&& ColorParamName != NAME_None
		&& ResolveVisualElementColor(ResolveVisualElementTag(SkillAbility, MeshComp), VisualElementColor);

	UNiagaraComponent* SpawnedVFX = nullptr;
	if (bSpawnInWorld)
	{
		const FTransform SocketTransform = (SocketName != NAME_None)
			? AttachComponent->GetSocketTransform(SocketName)
			: AttachComponent->GetComponentTransform();
		const FTransform OffsetTransform(RotationOffset, LocationOffset, SpawnScale);
		const FTransform SpawnTransform = OffsetTransform * SocketTransform;

		SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			NiagaraTemplate,
			SpawnTransform.GetLocation(),
			SpawnTransform.Rotator(),
			SpawnTransform.GetScale3D(),
			true,
			!bHasVisualElementColor,
			ENCPoolMethod::None,
			true);
	}
	else
	{
		SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraTemplate,
			AttachComponent,
			SocketName,
			LocationOffset,
			RotationOffset,
			SpawnScale,
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::None,
			!bHasVisualElementColor);
	}

	if (!SpawnedVFX || !bHasVisualElementColor) return;

	SpawnedVFX->SetVariableLinearColor(ColorParamName, VisualElementColor);
	SpawnedVFX->Activate(true);
}

FString UAnimNotify_SpawnNiagara::GetNotifyName_Implementation() const
{
	return TEXT("Spawn Niagara");
}
