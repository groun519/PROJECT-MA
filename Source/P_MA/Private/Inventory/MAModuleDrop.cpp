#include "Inventory/MAModuleDrop.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractableComponent.h"
#include "GAS/Skill/Addon/Item/MASkillModuleItemAddon.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "Inventory/MAInventoryComponent.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "Player/MAPlayerCharacter.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

AMAModuleDrop::AMAModuleDrop()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	InteractableComponent = CreateDefaultSubobject<UMAInteractableComponent>(TEXT("InteractableComponent"));
	RootComponent = InteractableComponent;
	InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);
	InteractableComponent->CALL_SETUP_CURSOR_HOVER(HandleCursorHover);
	InteractableComponent->CALL_SETUP_INTERACTION_MODE(Server);

	DropVisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DropVisualRoot"));
	DropVisualRoot->SetupAttachment(RootComponent);

	DropMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DropMeshComponent"));
	DropMeshComponent->SetupAttachment(DropVisualRoot);
	DropMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractableComponent->AddCursorHoverTarget(DropMeshComponent);

	HighlightComponent = CreateDefaultSubobject<UMAHighlightComponent>(TEXT("HighlightComponent"));
	HighlightComponent->AddTarget(DropMeshComponent);
	InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);

	RarityVisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RarityVisualComponent"));
	RarityVisualComponent->SetupAttachment(DropVisualRoot);
	RarityVisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TooltipWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidgetComponent"));
	TooltipWidgetComponent->SetupAttachment(RootComponent);
	TooltipWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	TooltipWidgetComponent->SetDrawAtDesiredSize(true);
	TooltipWidgetComponent->SetVisibility(false);
}

void AMAModuleDrop::BeginPlay()
{
	Super::BeginPlay();
	TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
	TooltipWidgetComponent->InitWidget();
	RefreshPresentation();
	if (DropData.bInFlight) StartDropAnimation();
}

void AMAModuleDrop::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bDropAnimationActive) return;

	DropAnimationElapsed += DeltaSeconds;
	const float Alpha = DropAnimationDuration > 0.f
		? FMath::Clamp(DropAnimationElapsed / DropAnimationDuration, 0.f, 1.f)
		: 1.f;
	const FVector Location = FMath::Lerp(
		static_cast<FVector>(DropData.VisualOrigin),
		GetActorLocation(),
		Alpha) + FVector::UpVector * (4.f * Alpha * (1.f - Alpha) * DropArcHeight);
	DropVisualRoot->SetWorldLocation(Location);

	if (Alpha >= 1.f) FinishDropAnimation();
}

void AMAModuleDrop::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAModuleDrop, DropData);
}

bool AMAModuleDrop::InitializeDrop(const int32 ModuleId, const int32 Count)
{
	if (!HasAuthority() || ModuleId <= 0 || Count <= 0) return false;

	CachedModule = UMASkillModule::LoadById(ModuleId);
	if (!CachedModule) return false;

	DropData.ModuleId = ModuleId;
	DropData.Count = Count;
	RefreshPresentation();
	if (DropData.bInFlight) StartDropAnimation();
	ForceNetUpdate();
	return true;
}

void AMAModuleDrop::DropNear(const FVector& Origin, const int32 DropCount)
{
	if (!HasAuthority()) return;

	SetActorLocation(FindDropLocationNear(Origin, DropCount));
	DropData.VisualOrigin = Origin;
	DropData.bInFlight = DropAnimationDuration > 0.f;
	RefreshPresentation();
	if (DropData.bInFlight) StartDropAnimation();
	else FinishDropAnimation();
	ForceNetUpdate();
}

void AMAModuleDrop::HandleInteract(AMAPlayerCharacter* Interactor)
{
	if (!HasAuthority() || bPickupInProgress || !Interactor || !DropData.IsValid()) return;

	UMAInventoryComponent* Inventory = Interactor->GetInventoryComponent();
	if (!Inventory) return;

	bPickupInProgress = true;
	if (Inventory->RequestAddModule(DropData.ModuleId, DropData.Count))
	{
		Destroy();
		return;
	}
	bPickupInProgress = false;
}

void AMAModuleDrop::HandleCursorHover(const bool bHovered)
{
	TooltipWidgetComponent->SetVisibility(
		bHovered && CachedModule && TooltipWidgetComponent->GetUserWidgetObject());
}

FVector AMAModuleDrop::FindDropLocationNear(const FVector& Origin, const int32 DropCount) const
{
	UWorld* World = GetWorld();
	if (!World) return Origin;

	static constexpr int32 MaxPlacementAttempts = 64;
	static constexpr float TraceUp = 200.f;
	static constexpr float TraceDown = 1000.f;
	const int32 SafeDropCount = FMath::Max(DropCount, 1);
	const float MaxDropDistance = BaseDropDistance
		+ DropDistancePerItem * static_cast<float>(SafeDropCount - 1);
	const float SearchRadiusSquared = FMath::Square(MaxDropDistance + BaseDropDistance);
	const float SpacingSquared = FMath::Square(BaseDropDistance);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlaceModuleDrop), false, this);
	TArray<FVector> OccupiedLocations;
	for (const AMAModuleDrop* Drop : TActorRange<AMAModuleDrop>(World))
	{
		if (Drop == this || Drop->IsActorBeingDestroyed() || !Drop->DropData.IsValid()) continue;
		QueryParams.AddIgnoredActor(Drop);
		if (FVector::DistSquared2D(Drop->GetActorLocation(), Origin) <= SearchRadiusSquared)
		{
			OccupiedLocations.Add(Drop->GetActorLocation());
		}
	}

	const FCollisionObjectQueryParams GroundObjectTypes(ECC_WorldStatic);
	FVector FallbackLocation = Origin;
	bool bFoundGround = false;
	for (int32 Attempt = 0; Attempt < MaxPlacementAttempts; ++Attempt)
	{
		const float Radius = FMath::FRandRange(BaseDropDistance, MaxDropDistance);
		const float Angle = FMath::FRandRange(0.f, UE_TWO_PI);
		const FVector Candidate = Origin + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

		FHitResult GroundHit;
		if (!World->LineTraceSingleByObjectType(
			GroundHit,
			Candidate + FVector::UpVector * TraceUp,
			Candidate - FVector::UpVector * TraceDown,
			GroundObjectTypes,
			QueryParams))
		{
			continue;
		}

		const FVector GroundLocation = GroundHit.ImpactPoint;
		if (!bFoundGround)
		{
			FallbackLocation = GroundLocation;
			bFoundGround = true;
		}
		if (!OccupiedLocations.ContainsByPredicate([&GroundLocation, SpacingSquared](const FVector& Location)
		{
			return FVector::DistSquared(Location, GroundLocation) < SpacingSquared;
		}))
		{
			return GroundLocation;
		}
	}

	return FallbackLocation;
}

void AMAModuleDrop::StartDropAnimation()
{
	if (bDropAnimationActive || !DropData.bInFlight || !DropData.IsValid()) return;

	bDropAnimationActive = true;
	DropAnimationElapsed = 0.f;
	InteractableComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TooltipWidgetComponent->SetVisibility(false);
	if (FlightMesh.Mesh)
	{
		ApplyMeshVisual(&FlightMesh);
		if (const FMAModuleRarityData* RarityData = ResolveRarityData())
		{
			static const FName BaseColorParameterName(TEXT("BaseColor"));
			DropMeshComponent->SetColorParameterValueOnMaterials(BaseColorParameterName, RarityData->Color);
		}
	}
	DropVisualRoot->SetWorldLocation(static_cast<FVector>(DropData.VisualOrigin));
	SetActorTickEnabled(true);
}

void AMAModuleDrop::FinishDropAnimation()
{
	bDropAnimationActive = false;
	DropVisualRoot->SetRelativeLocation(FVector::ZeroVector);
	SetActorTickEnabled(false);

	if (HasAuthority() && DropData.bInFlight)
	{
		DropData.bInFlight = false;
		ForceNetUpdate();
	}
	RefreshPresentation();
}

void AMAModuleDrop::ApplyMeshVisual(const FMAStaticMeshVisualData* VisualData)
{
	DropMeshComponent->EmptyOverrideMaterials();
	DropMeshComponent->SetStaticMesh(VisualData ? VisualData->Mesh : nullptr);
	DropMeshComponent->SetRelativeTransform(
		VisualData ? VisualData->TransformOffset : FTransform::Identity);
}

void AMAModuleDrop::RefreshPresentation()
{
	if (!DropData.IsValid())
	{
		CachedModule = nullptr;
	}
	else if (!CachedModule || CachedModule->GetModuleId() != DropData.ModuleId)
	{
		CachedModule = UMASkillModule::LoadById(DropData.ModuleId);
	}
	const bool bCanInteract = CachedModule && !DropData.bInFlight;
	InteractableComponent->SetCollisionEnabled(bCanInteract
		? ECollisionEnabled::QueryOnly
		: ECollisionEnabled::NoCollision);
	DropMeshComponent->SetCollisionEnabled(bCanInteract
		? ECollisionEnabled::QueryOnly
		: ECollisionEnabled::NoCollision);
	if (GetNetMode() == NM_DedicatedServer) return;

	ApplyMeshVisual(ResolveDropMesh());
	RefreshTooltip();

	const FMAModuleRarityData* RarityData = ResolveRarityData();
	RarityVisualComponent->SetVisibility(RarityData != nullptr);
	if (!RarityData) return;

	static const FName BaseColorParameterName(TEXT("BaseColor"));
	DropMeshComponent->SetColorParameterValueOnMaterials(BaseColorParameterName, RarityData->Color);

	static const FName ColorParameterName(TEXT("RarityColor"));
	static const FName IntensityParameterName(TEXT("RarityIntensity"));
	RarityVisualComponent->SetColorParameterValueOnMaterials(ColorParameterName, RarityData->Color);
	RarityVisualComponent->SetScalarParameterValueOnMaterials(IntensityParameterName, RarityData->GlowAlpha);
}

void AMAModuleDrop::RefreshTooltip()
{
	UMASkillTooltipWidget* TooltipWidget =
		Cast<UMASkillTooltipWidget>(TooltipWidgetComponent->GetUserWidgetObject());
	if (!TooltipWidget) return;

	FMADisplayData DisplayData = CachedModule
		? CachedModule->ResolveDisplayData(UMAGameSettings::Get()->GetModuleQualityData())
		: FMADisplayData();
	if (DropData.Count > 1)
	{
		DisplayData.DisplayName = FText::Format(
			NSLOCTEXT("MASkillTooltipWidget", "ItemCountFormat", "{0} x{1}"),
			DisplayData.DisplayName,
			FText::AsNumber(DropData.Count));
	}
	TooltipWidget->SetDisplayData(DisplayData);
}

const FMAStaticMeshVisualData* AMAModuleDrop::ResolveDropMesh() const
{
	if (!CachedModule) return nullptr;

	switch (CachedModule->GetModuleType())
	{
	case EMASkillModuleType::Module:
		return &ModuleMesh;
	case EMASkillModuleType::Sub:
		return &SubModuleMesh;
	case EMASkillModuleType::Item:
		if (const UMASkillModuleItemAddon* ItemAddon =
			CachedModule->FindAddon<UMASkillModuleItemAddon>())
		{
			if (ItemAddon->GetWorldMesh().Mesh) return &ItemAddon->GetWorldMesh();
		}
		return &DefaultItemMesh;
	default:
		return nullptr;
	}
}

const FMAModuleRarityData* AMAModuleDrop::ResolveRarityData() const
{
	const UMAModuleQualityData* QualityData = UMAGameSettings::Get()->GetModuleQualityData();
	return CachedModule && QualityData
		? QualityData->FindRarityData(CachedModule->GetModuleQuality().Rarity)
		: nullptr;
}

void AMAModuleDrop::OnRep_DropData()
{
	RefreshPresentation();
	if (DropData.bInFlight) StartDropAnimation();
	else FinishDropAnimation();
}
