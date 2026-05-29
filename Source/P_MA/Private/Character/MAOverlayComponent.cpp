#include "Character/MAOverlayComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"

UMAOverlayComponent::UMAOverlayComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMAOverlayComponent::BeginPlay()
{
	Super::BeginPlay();

	TargetMesh = ResolveTargetMesh();
}

void UMAOverlayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool bChanged = false;
	for (int32 Index = OverlayRequests.Num() - 1; Index >= 0; --Index)
	{
		FOverlayRequest& Request = OverlayRequests[Index];
		if (Request.RemainingTime < 0.f) continue;

		Request.RemainingTime -= DeltaTime;
		if (Request.RemainingTime <= 0.f)
		{
			OverlayRequests.RemoveAt(Index);
			bChanged = true;
		}
	}

	if (bChanged)
	{
		RefreshActiveOverlay();
	}
	RefreshTickEnabled();
}

UMaterialInstanceDynamic* UMAOverlayComponent::AddTimedOverlay(UMaterialInterface* Material, int32 Priority, float Duration)
{
	if (!Material || Duration <= 0.f) return nullptr;
	return AddOverlay(Material, Priority, Duration);
}

UMaterialInstanceDynamic* UMAOverlayComponent::AddPersistentOverlay(UMaterialInterface* Material, int32 Priority)
{
	if (!Material) return nullptr;

	for (const FOverlayRequest& Request : OverlayRequests)
	{
		if (Request.Material == Material && Request.RemainingTime < 0.f)
		{
			return Request.MID;
		}
	}

	return AddOverlay(Material, Priority, -1.f);
}

void UMAOverlayComponent::RemovePersistentOverlay(UMaterialInterface* Material)
{
	if (!Material) return;

	if (OverlayRequests.RemoveAll([Material](const FOverlayRequest& Request)
	{
		return Request.Material == Material && Request.RemainingTime < 0.f;
	}) <= 0) return;

	RefreshActiveOverlay();
	RefreshTickEnabled();
}

UMaterialInstanceDynamic* UMAOverlayComponent::AddOverlay(UMaterialInterface* Material, int32 Priority, float RemainingTime)
{
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Material, this);
	if (!MID) return nullptr;

	FOverlayRequest& Request = OverlayRequests.AddDefaulted_GetRef();
	Request.Material = Material;
	Request.MID = MID;
	Request.Priority = Priority;
	Request.RemainingTime = RemainingTime;

	RefreshActiveOverlay();
	RefreshTickEnabled();
	return MID;
}

void UMAOverlayComponent::RefreshActiveOverlay()
{
	if (!TargetMesh)
	{
		TargetMesh = ResolveTargetMesh();
	}
	if (!TargetMesh) return;

	UMaterialInstanceDynamic* SelectedMID = nullptr;
	int32 SelectedPriority = MAX_int32;
	for (const FOverlayRequest& Request : OverlayRequests)
	{
		if (!Request.MID || Request.Priority >= SelectedPriority) continue;

		SelectedMID = Request.MID;
		SelectedPriority = Request.Priority;
	}

	if (ActiveMID == SelectedMID) return;

	TargetMesh->SetOverlayMaterial(SelectedMID);
	ActiveMID = SelectedMID;
}

void UMAOverlayComponent::RefreshTickEnabled()
{
	SetComponentTickEnabled(OverlayRequests.ContainsByPredicate([](const FOverlayRequest& Request)
	{
		return Request.RemainingTime >= 0.f;
	}));
}

USkeletalMeshComponent* UMAOverlayComponent::ResolveTargetMesh() const
{
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		return Character->GetMesh();
	}

	return GetOwner() ? GetOwner()->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
}
