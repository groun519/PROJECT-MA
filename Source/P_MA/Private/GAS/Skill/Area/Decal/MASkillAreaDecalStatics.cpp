#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"

#include "Character/MACharacter.h"
#include "Components/DecalComponent.h"
#include "Engine/DataTable.h"
#include "GAS/Skill/MAElementData.h"
#include "GAS/Skill/Area/Decal/MAAreaDecalData.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillGenericDataAsset.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GameplayTagContainer.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"
#include "TimerManager.h"

static constexpr float ImpactFadeDuration = 0.5f;
static constexpr float ImpactInnerAlpha = 5.f;
static constexpr float DecalProjectionDepth = 110.f;
static constexpr float DecalZOffset = -100.f;

static const UMASkillGenericDataAsset* ResolveDefaultGenericData()
{
	const UMAGameSettings* GameSettings = UMAGameSettings::Get();
	return GameSettings ? GameSettings->GetDefaultSkillGenericDataAsset() : nullptr;
}

static FName ResolveElementRowName(const FGameplayTag& ElementalTag)
{
	if (!ElementalTag.IsValid()) return NAME_None;

	FString RowNameString = ElementalTag.GetTagName().ToString();
	return RowNameString.Split(TEXT("."), nullptr, &RowNameString, ESearchCase::CaseSensitive, ESearchDir::FromEnd)
		? FName(*RowNameString)
		: NAME_None;
}

static FName ResolveDecalRowName(EMASkillAreaShape Shape)
{
	switch (Shape)
	{
	case EMASkillAreaShape::Circle: return TEXT("Circle");
	case EMASkillAreaShape::Rect: return TEXT("Rect");
	case EMASkillAreaShape::Line: return TEXT("Line");
	default: return NAME_None;
	}
}

static FVector ResolveDecalSize(const FMASkillWorldCircleArea& Circle)
{
	return FVector(DecalProjectionDepth, Circle.Radius, Circle.Radius);
}

static FVector ResolveDecalSize(const FMASkillWorldRectArea& Rect)
{
	return FVector(DecalProjectionDepth, Rect.Width * 0.5f, Rect.Height * 0.5f);
}

static FVector ResolveDecalSize(const FMASkillWorldLineArea& Line)
{
	return FVector(DecalProjectionDepth, Line.Width * 0.5f, Line.Length * 0.5f);
}

static FVector ResolveDecalSize(const FMASkillWorldAreaShape& Area)
{
	switch (Area.Shape)
	{
	case EMASkillAreaShape::Circle:
		return ResolveDecalSize(Area.Circle);
	case EMASkillAreaShape::Rect:
		return ResolveDecalSize(Area.Rect);
	case EMASkillAreaShape::Line:
		return ResolveDecalSize(Area.Line);
	default:
		return FVector::ZeroVector;
	}
}

static FLinearColor ResolveElementColor(
	const UMASkillAbility* SkillAbility,
	const AActor* ComponentOwner,
	const UDataTable* ElementalDataTable)
{
	FGameplayTag ElementalTag;
	if (SkillAbility)
	{
		ElementalTag = SkillAbility->GetElementalTag();
	}
	else if (const AMACharacter* Character = Cast<AMACharacter>(ComponentOwner))
	{
		if (const UMASkillManagerComponent* SkillManager = Character->GetSkillManagerComponent())
		{
			ElementalTag = SkillManager->GetActivePreviewElementalTag();
		}
	}

	const FName ElementRowName = ResolveElementRowName(ElementalTag);
	if (ElementRowName == NAME_None || !ElementalDataTable) return FLinearColor::White;

	const FMAElementDataRow* ElementRow = ElementalDataTable->FindRow<FMAElementDataRow>(
		ElementRowName,
		TEXT("MASkillAreaDecal"));
	return ElementRow ? ElementRow->ElementColor : FLinearColor::White;
}

UDecalComponent* MASkillAreaDecalStatics::Spawn(
	AActor* ComponentOwner,
	USceneComponent* AttachParent,
	const UMASkillAbility* SkillAbility,
	const FMASkillWorldAreaShape& Area)
{
	UWorld* World = ComponentOwner ? ComponentOwner->GetWorld() : nullptr;
	if (!World || World->GetNetMode() == NM_DedicatedServer || !Area.IsValid()) return nullptr;

	const UDataTable* DecalDataTable = SkillAbility ? SkillAbility->GetAreaDecalDataTable() : nullptr;
	const UDataTable* ElementalDataTable = SkillAbility ? SkillAbility->GetElementalDataTable() : nullptr;
	if (!DecalDataTable || !ElementalDataTable)
	{
		const UMASkillGenericDataAsset* GenericData = ResolveDefaultGenericData();
		if (!DecalDataTable && GenericData) DecalDataTable = GenericData->GetAreaDecalDataTable();
		if (!ElementalDataTable && GenericData) ElementalDataTable = GenericData->GetElementalDataTable();
	}

	const FName DecalRowName = ResolveDecalRowName(Area.Shape);
	if (!DecalDataTable || DecalRowName == NAME_None) return nullptr;

	const FMAAreaDecalDataRow* DecalRow = DecalDataTable->FindRow<FMAAreaDecalDataRow>(
		DecalRowName,
		TEXT("MASkillAreaDecal"));
	if (!DecalRow || !DecalRow->DecalMaterial) return nullptr;

	UDecalComponent* Decal = NewObject<UDecalComponent>(ComponentOwner);
	ComponentOwner->AddInstanceComponent(Decal);
	Decal->SetDecalMaterial(DecalRow->DecalMaterial);
	Decal->DecalSize = ResolveDecalSize(Area);
	Decal->RegisterComponent();
	if (AttachParent) Decal->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepWorldTransform);
	SetAreaTransform(*Decal, Area);

	UMaterialInstanceDynamic* AreaMID = Decal->CreateDynamicMaterialInstance();
	if (!AreaMID) return Decal;

	AreaMID->SetVectorParameterValue(
		PARAM_AreaDecal_BaseColor,
		ResolveElementColor(SkillAbility, ComponentOwner, ElementalDataTable));

	if (Area.Shape == EMASkillAreaShape::Rect || Area.Shape == EMASkillAreaShape::Line)
	{
		AreaMID->SetScalarParameterValue(PARAM_AreaDecal_Width, 1.f);
		AreaMID->SetScalarParameterValue(PARAM_AreaDecal_Height, 1.f);
	}

	if (Area.Shape == EMASkillAreaShape::Circle)
	{
		const bool bSector = Area.Circle.bUseSector && Area.Circle.SectorAngle > 0.f && Area.Circle.SectorAngle < 360.f;
		const float SectorHalfAngle = Area.Circle.SectorAngle * 0.5f;
		AreaMID->SetScalarParameterValue(PARAM_AreaDecal_BaseAngle, bSector ? -SectorHalfAngle : 0.f);
		AreaMID->SetScalarParameterValue(PARAM_AreaDecal_EndAngle, bSector ? SectorHalfAngle : 360.f);
	}
	return Decal;
}

void MASkillAreaDecalStatics::SetAreaTransform(
	UDecalComponent& Decal,
	const FMASkillWorldAreaShape& Area)
{
	Decal.SetWorldLocationAndRotation(
		Area.Center + FVector::UpVector * DecalZOffset,
		Area.GetDecalRotation());
}

void MASkillAreaDecalStatics::SpawnImpact(
	UMASkillAbility& SkillAbility,
	const FMASkillWorldAreaShape& Area)
{
	AActor* AvatarActor = SkillAbility.GetAvatarActorFromActorInfo();
	SpawnImpact(AvatarActor, &SkillAbility, Area);
}

void MASkillAreaDecalStatics::SpawnImpact(
	AActor* ComponentOwner,
	const UMASkillAbility* SkillAbility,
	const FMASkillWorldAreaShape& Area)
{
	UDecalComponent* Decal = Spawn(
		ComponentOwner,
		nullptr,
		SkillAbility,
		Area);
	if (!Decal) return;

	if (UMaterialInstanceDynamic* AreaMID = Cast<UMaterialInstanceDynamic>(Decal->GetDecalMaterial()))
	{
		AreaMID->SetScalarParameterValue(PARAM_AreaDecal_InnerAlpha, ImpactInnerAlpha);
	}

	Decal->SetFadeOut(0.f, ImpactFadeDuration, false);
	if (UWorld* World = ComponentOwner ? ComponentOwner->GetWorld() : nullptr)
	{
		FTimerHandle DestroyTimerHandle;
		TWeakObjectPtr<UDecalComponent> WeakDecal = Decal;
		World->GetTimerManager().SetTimer(
			DestroyTimerHandle,
			[WeakDecal]()
			{
				if (UDecalComponent* DecalComponent = WeakDecal.Get())
				{
					DecalComponent->DestroyComponent();
				}
			},
			ImpactFadeDuration,
			false);
	}
}
