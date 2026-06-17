#include "Animation/Notify/AnimNotify_SpawnNiagara.h"

#include "Animation/Notify/Skill/MASkillAnimNotifyStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/MASkillAbility.h"
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

static bool ResolveElementalColor(const UMASkillAbility* SkillAbility, FLinearColor& OutColor)
{
	if (!SkillAbility) return false;

	const UDataTable* ElementalDataTable = SkillAbility->GetElementalDataTable();
	const FGameplayTag& ElementalTag = SkillAbility->GetElementalTag();
	if (!ElementalDataTable || !ElementalTag.IsValid()) return false;

	FString UnusedRoot;
	FString RowNameString;
	if (!ElementalTag.ToString().Split(TEXT("."), &UnusedRoot, &RowNameString, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		return false;
	}

	const FMAElementDataRow* ElementRow = ElementalDataTable->FindRow<FMAElementDataRow>(FName(*RowNameString), TEXT("AnimNotify_SpawnNiagara"));
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

	UMASkillAbility* SkillAbility = (bApplySkillAreaScale || bApplyElementalColor)
		? MASkillAnimNotifyStatics::ResolveAnimationOwnerSkillAbility(MeshComp, Animation)
		: nullptr;
	const FVector SpawnScale = bApplySkillAreaScale
		? Scale * MASkillAnimNotifyStatics::ResolveSkillAreaScale(SkillAbility)
		: Scale;

	UNiagaraComponent* SpawnedVFX = nullptr;
#if WITH_EDITOR
	if (World->WorldType == EWorldType::EditorPreview)
	{
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
				SpawnTransform.GetScale3D());
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
				true);
		}
	}
	else
#endif
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
			SpawnTransform.GetScale3D());
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
			true);
	}

	if (!SpawnedVFX || !bApplyElementalColor || ColorParamName == NAME_None) return;
	if (!SkillAbility) return;

	FLinearColor ElementalColor = FLinearColor::White;
	if (!ResolveElementalColor(SkillAbility, ElementalColor)) return;

	SpawnedVFX->SetVariableLinearColor(ColorParamName, ElementalColor);
}

FString UAnimNotify_SpawnNiagara::GetNotifyName_Implementation() const
{
	return TEXT("Spawn Niagara");
}
