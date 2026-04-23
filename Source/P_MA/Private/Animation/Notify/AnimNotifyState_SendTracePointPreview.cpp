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

const UMASkillAbility* UAnimNotifyState_SendTracePointPreview::ResolveAnimationOwnerSkillAbility(USkeletalMeshComponent* MeshComp, const UAnimSequenceBase* Animation)
{
	if (!MeshComp || !Animation) return nullptr;

	const UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(MeshComp->GetAnimInstance());
	return AnimInstance ? AnimInstance->FindAnimationOwner(Animation) : nullptr;
}

FName UAnimNotifyState_SendTracePointPreview::ResolveElementRowName(const FGameplayTag& ElementalTag)
{
	if (!ElementalTag.IsValid()) return NAME_None;

	FString RowNameString = ElementalTag.GetTagName().ToString();
	if (!RowNameString.Split(TEXT("."), nullptr, &RowNameString, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
	{
		return NAME_None;
	}

	return FName(*RowNameString);
}

bool UAnimNotifyState_SendTracePointPreview::ResolvePreviewWorldSpace(USkeletalMeshComponent* MeshComp, FVector& OutWorldLocation, FVector& OutShapeForward, FRotator& OutDecalRotation) const
{
	FQuat WorldRotation = FQuat::Identity;
	FVector MeshForward = FVector::ZeroVector;
	if (!MATracePointNotify::ResolveWorldSpace(MeshComp, LocalOffset, LocalRotation, OutWorldLocation, WorldRotation, MeshForward)) return false;

	OutShapeForward = Shape == EVA_Shape::Line ? WorldRotation.GetAxisY().GetSafeNormal2D() : MeshForward;
	if (Shape == EVA_Shape::Line)
	{
		OutWorldLocation += OutShapeForward * (Length * 0.5f);
		OutDecalRotation = FRotator(-90.f, OutShapeForward.Rotation().Yaw + 90.f, 0.f);
		return true;
	}

	OutDecalRotation = FRotator(-90.f, WorldRotation.Rotator().Yaw + 90.f, 0.f);
	return true;
}

FName UAnimNotifyState_SendTracePointPreview::GetPreviewDecalRowName() const
{
	return Shape == EVA_Shape::Circle ? FName(TEXT("Circle"))
		: Shape == EVA_Shape::Rect ? FName(TEXT("Rect"))
		: Shape == EVA_Shape::Line ? FName(TEXT("Line"))
		: NAME_None;
}

FVector UAnimNotifyState_SendTracePointPreview::GetPreviewDecalSize() const
{
	return Shape == EVA_Shape::Circle ? FVector(10.f, Radius, Radius)
		: Shape == EVA_Shape::Rect ? FVector(10.f, Width, Height)
		: Shape == EVA_Shape::Line ? FVector(10.f, Length * 0.5f, Radius)
		: FVector::ZeroVector;
}

void UAnimNotifyState_SendTracePointPreview::ConfigurePreviewDecalMaterial(UMaterialInstanceDynamic* DecalMID, const FLinearColor& ElementColor) const
{
	if (!DecalMID) return;

	DecalMID->SetVectorParameterValue(TEXT("BaseColor"), ElementColor);
	if (Shape == EVA_Shape::Circle)
	{
		const bool bSector = bUseSector && SectorAngle > 0.f && SectorAngle < 360.f;
		const float SectorHalfAngle = SectorAngle * 0.5f;
		DecalMID->SetScalarParameterValue(TEXT("_BaseAngle"), bSector ? -SectorHalfAngle : 0.f);
		DecalMID->SetScalarParameterValue(TEXT("_EndAngle"), bSector ? SectorHalfAngle : 360.f);
		return;
	}

	if (Shape == EVA_Shape::Rect || Shape == EVA_Shape::Line)
	{
		DecalMID->SetScalarParameterValue(TEXT("_Width"), 1.f);
		DecalMID->SetScalarParameterValue(TEXT("_Height"), 1.f);
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
	FVector ShapeForward = FVector::ZeroVector;
	FRotator DecalRotation = FRotator::ZeroRotator;
	if (!ResolvePreviewWorldSpace(MeshComp, WorldLocation, ShapeForward, DecalRotation)) return;

	MATracePointNotify::DrawDebugShape(World, Shape, WorldLocation, ShapeForward, Radius, bUseSector, SectorAngle, Width, Height, Length, DebugColor, DebugThickness);

	if (World->GetNetMode() == NM_DedicatedServer || MATracePointNotify::IsEditorPreviewWorldNoPIE(World)) return;
	SpawnPreviewDecal(MeshComp, Animation, WorldLocation, DecalRotation);
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
	FVector ShapeForward = FVector::ZeroVector;
	FRotator DecalRotation = FRotator::ZeroRotator;
	if (!ResolvePreviewWorldSpace(MeshComp, WorldLocation, ShapeForward, DecalRotation)) return;

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
		Length,
		bIgnoreOwner,
		bDrawDebug,
		TriggerGameplayCueTags,
		WorldLocation);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Data);
}

void UAnimNotifyState_SendTracePointPreview::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World || MATracePointNotify::IsEditorPreviewWorldNoPIE(World)) return;

	FVector WorldLocation = FVector::ZeroVector;
	FVector ShapeForward = FVector::ZeroVector;
	FRotator DecalRotation = FRotator::ZeroRotator;
	if (!ResolvePreviewWorldSpace(MeshComp, WorldLocation, ShapeForward, DecalRotation)) return;
	UpdatePreviewDecalTransform(MeshComp, WorldLocation, DecalRotation);
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
	const FVector& WorldLocation, const FRotator& DecalRotation)
{
	DestroyPreviewDecal(MeshComp);
	if (!MeshComp) return;

	const UMASkillAbility* SkillAbility = ResolveAnimationOwnerSkillAbility(MeshComp, Animation);
	if (!SkillAbility) return;

	const UDataTable* OverlapDecalDataTable = SkillAbility->GetOverlapDecalDataTable();
	if (!OverlapDecalDataTable) return;

	const FName DecalRowName = GetPreviewDecalRowName();
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

	const FVector DecalSize = GetPreviewDecalSize();
	if (DecalSize.IsNearlyZero()) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	UDecalComponent* DecalComponent = bSpawnInWorld
		? UGameplayStatics::SpawnDecalAtLocation(World, DecalRow->DecalMaterial, DecalSize, WorldLocation, DecalRotation, 0.f)
		: UGameplayStatics::SpawnDecalAttached(DecalRow->DecalMaterial, DecalSize, MeshComp, NAME_None, WorldLocation, DecalRotation, EAttachLocation::KeepWorldPosition, 0.f);
	if (!DecalComponent) return;

	ConfigurePreviewDecalMaterial(DecalComponent->CreateDynamicMaterialInstance(), ElementColor);
	ActiveDecals.Add(MeshComp, DecalComponent);
	UpdatePreviewDecalTransform(MeshComp, WorldLocation, DecalRotation);
}

void UAnimNotifyState_SendTracePointPreview::UpdatePreviewDecalTransform(USkeletalMeshComponent* MeshComp, const FVector& WorldLocation, const FRotator& DecalRotation)
{
	TWeakObjectPtr<UDecalComponent>* FoundDecal = ActiveDecals.Find(MeshComp);
	if (!FoundDecal) return;

	UDecalComponent* DecalComponent = FoundDecal->Get();
	if (!DecalComponent) return;

	DecalComponent->SetWorldLocationAndRotation(WorldLocation, DecalRotation);
}
