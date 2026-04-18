#include "Animation/Notify/AnimNotifyState_SendTracePointPreview.h"

#include "Animation/MAAnimInstance.h"
#include "Animation/Notify/MATracePointNotifyHelper.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/DecalComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/MAOverlapDecalData.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
	const UMASkillAbility* ResolveAnimationOwnerSkillAbility(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation)
	{
		if (!MeshComp || !Animation) return nullptr;

		const UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(MeshComp->GetAnimInstance());
		return AnimInstance ? AnimInstance->FindAnimationOwner(Animation) : nullptr;
	}

	FName ResolveElementRowName(const FGameplayTag& ElementalTag)
	{
		if (!ElementalTag.IsValid()) return NAME_None;

		FString RowNameString = ElementalTag.GetTagName().ToString();
		if (!RowNameString.Split(TEXT("."), nullptr, &RowNameString, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
		{
			return NAME_None;
		}

		return FName(*RowNameString);
	}
}

void UAnimNotifyState_SendTracePointPreview::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	FVector WorldLocation = FVector::ZeroVector;
	FQuat WorldRotation = FQuat::Identity;
	FVector MeshForward = FVector::ZeroVector;
	if (!MATracePointNotify::ResolveWorldSpace(MeshComp, LocalOffset, LocalRotation, WorldLocation, WorldRotation, MeshForward))
	{
		return;
	}

	MATracePointNotify::DrawDebugShape(World, Shape, WorldLocation, MeshForward, Radius, bUseSector, SectorAngle, Width, Height, DebugColor, DebugThickness);

	if (World->GetNetMode() == NM_DedicatedServer || MATracePointNotify::IsEditorPreviewWorldNoPIE(World))
	{
		return;
	}

	SpawnPreviewDecal(MeshComp, Animation, WorldLocation, WorldRotation);
}

void UAnimNotifyState_SendTracePointPreview::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!MeshComp) return;

	DestroyPreviewDecal(MeshComp);

	UWorld* World = MeshComp->GetWorld();
	if (!World || MATracePointNotify::IsEditorPreviewWorldNoPIE(World)) return;

	FVector WorldLocation = FVector::ZeroVector;
	FVector UnusedMeshForward = FVector::ZeroVector;
	if (!MATracePointNotify::ResolveWorldSpace(MeshComp, LocalOffset, LocalRotation, WorldLocation, UnusedMeshForward))
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;

	FGameplayEventData Data;
	MATracePointNotify::AppendTargetData(
		Data,
		Owner,
		Shape,
		LocalOffset,
		LocalRotation,
		Radius,
		bUseSector,
		SectorAngle,
		Width,
		Height,
		bIgnoreOwner,
		bDrawDebug,
		TriggerGameplayCueTags,
		WorldLocation);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
}

FString UAnimNotifyState_SendTracePointPreview::GetNotifyName_Implementation() const
{
	return TEXT("Send Trace Point Preview");
}

void UAnimNotifyState_SendTracePointPreview::DestroyPreviewDecal(USkeletalMeshComponent* MeshComp)
{
	TWeakObjectPtr<UDecalComponent>* FoundDecal = ActiveDecals.Find(MeshComp);
	if (!FoundDecal) return;

	if (UDecalComponent* DecalComponent = FoundDecal->Get())
	{
		DecalComponent->DestroyComponent();
	}

	ActiveDecals.Remove(MeshComp);
}

void UAnimNotifyState_SendTracePointPreview::SpawnPreviewDecal(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation,
	const FVector& WorldLocation, const FQuat& WorldRotation)
{
	DestroyPreviewDecal(MeshComp);
	if (!MeshComp) return;

	const UMASkillAbility* SkillAbility = ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	if (!SkillAbility) return;

	const UDataTable* OverlapDecalDataTable = SkillAbility->GetOverlapDecalDataTable();
	if (!OverlapDecalDataTable) return;

	const FName DecalRowName =
		Shape == EVA_Shape::Circle ? FName(TEXT("Circle"))
		: Shape == EVA_Shape::Rect ? FName(TEXT("Rect"))
		: NAME_None;
	if (DecalRowName == NAME_None) return;

	const FMAOverlapDecalDataRow* DecalRow = OverlapDecalDataTable->FindRow<FMAOverlapDecalDataRow>(DecalRowName, TEXT("AnimNotifyState_SendTracePointPreview"));
	if (!DecalRow || !DecalRow->DecalMaterial) return;

	FLinearColor ElementColor = FLinearColor::White;
	if (const UDataTable* ElementalDataTable = SkillAbility->GetElementalDataTable())
	{
		const FName ElementRowName = ResolveElementRowName(SkillAbility->GetElementalTag());
		if (ElementRowName != NAME_None)
		{
			if (const FMAElementDataRow* ElementRow = ElementalDataTable->FindRow<FMAElementDataRow>(ElementRowName, TEXT("AnimNotifyState_SendTracePointPreview")))
			{
				ElementColor = ElementRow->ElementColor;
			}
		}
	}

	const bool bSector = Shape == EVA_Shape::Circle && bUseSector && SectorAngle > 0.f && SectorAngle < 360.f;
	const float SectorHalfAngle = SectorAngle * 0.5f;
	const FRotator DecalRotation(-90.f, WorldRotation.Rotator().Yaw + 90.f, 0.f);
	const FVector DecalSize =
		Shape == EVA_Shape::Circle ? FVector(10.f, Radius, Radius)
		: Shape == EVA_Shape::Rect ? FVector(10.f, Width, Height)
		: FVector::ZeroVector;
	if (DecalSize.IsNearlyZero()) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	UDecalComponent* DecalComponent = nullptr;
	if (bSpawnInWorld)
	{
		DecalComponent = UGameplayStatics::SpawnDecalAtLocation(
			World,
			DecalRow->DecalMaterial,
			DecalSize,
			WorldLocation,
			DecalRotation,
			0.f);
	}
	else
	{
		const FName AttachPointName = MeshComp->GetNumBones() > 0 ? MeshComp->GetBoneName(0) : NAME_None;
		const FVector RelativeLocation(LocalOffset.X, LocalOffset.Y, 0.f);
		const FRotator RelativeRotation(-90.f, LocalRotation.Yaw + 90.f, 0.f);

		DecalComponent = UGameplayStatics::SpawnDecalAttached(
			DecalRow->DecalMaterial,
			DecalSize,
			MeshComp,
			AttachPointName,
			RelativeLocation,
			RelativeRotation,
			EAttachLocation::KeepRelativeOffset,
			0.f);
	}

	if (!DecalComponent) return;

	if (UMaterialInstanceDynamic* DecalMID = DecalComponent->CreateDynamicMaterialInstance())
	{
		DecalMID->SetVectorParameterValue(TEXT("BaseColor"), ElementColor);
		DecalMID->SetScalarParameterValue(TEXT("_BaseAngle"), bSector ? -SectorHalfAngle : 0.f);
		DecalMID->SetScalarParameterValue(TEXT("_EndAngle"), bSector ? SectorHalfAngle : 360.f);
	}

	ActiveDecals.Add(MeshComp, DecalComponent);
}
