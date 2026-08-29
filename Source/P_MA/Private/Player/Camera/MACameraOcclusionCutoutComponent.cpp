#include "Player/Camera/MACameraOcclusionCutoutComponent.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "UObject/ConstructorHelpers.h"

namespace MACameraOcclusionCutout
{
	const FName EnabledParameter(TEXT("CutoutEnabled"));
	const FName RadiusParameter(TEXT("CutoutRadius"));
	const FName CameraPositionParameter(TEXT("CameraPosition"));
	const FName TargetPositionParameter(TEXT("TargetPosition"));
}

UMACameraOcclusionCutoutComponent::UMACameraOcclusionCutoutComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> ParameterCollectionFinder(
		TEXT("/Game/_WorkSpace/Camera/OcclusionCutout/MPC_CameraOcclusionCutout"));
	ParameterCollection = ParameterCollectionFinder.Object;
}

void UMACameraOcclusionCutoutComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearTarget();
	Super::EndPlay(EndPlayReason);
}

void UMACameraOcclusionCutoutComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ViewerController.IsValid() || !RevealTargetActor.IsValid())
	{
		ClearTarget();
		return;
	}

	UpdateMaterialParameters();
}

void UMACameraOcclusionCutoutComponent::RevealTarget(APlayerController& Viewer, AActor& Target)
{
	if (!Viewer.IsLocalController() || !ParameterCollection) return;

	ViewerController = &Viewer;
	RevealTargetActor = &Target;
	SetComponentTickEnabled(true);
	UpdateMaterialParameters();
}

void UMACameraOcclusionCutoutComponent::ClearTarget()
{
	SetCutoutEnabled(false);
	SetComponentTickEnabled(false);
	ViewerController.Reset();
	RevealTargetActor.Reset();
}

void UMACameraOcclusionCutoutComponent::SetCutoutRadius(const float Radius)
{
	CutoutRadius = FMath::Max(Radius, 1.f);
	if (RevealTargetActor.IsValid()) UpdateMaterialParameters();
}

void UMACameraOcclusionCutoutComponent::UpdateMaterialParameters()
{
	APlayerController* Viewer = ViewerController.Get();
	AActor* Target = RevealTargetActor.Get();
	UWorld* World = GetWorld();
	if (!Viewer || !Target || !World || !Viewer->PlayerCameraManager || !ParameterCollection)
	{
		ClearTarget();
		return;
	}

	UMaterialParameterCollectionInstance* Parameters =
		World->GetParameterCollectionInstance(ParameterCollection);
	if (!Parameters)
	{
		ClearTarget();
		return;
	}

	FVector TargetPosition;
	FVector TargetExtent;
	Target->GetActorBounds(true, TargetPosition, TargetExtent);
	if (TargetExtent.IsNearlyZero())
	{
		TargetPosition = Target->GetActorLocation();
	}

	const FVector CameraPosition = Viewer->PlayerCameraManager->GetCameraLocation();
	Parameters->SetVectorParameterValue(
		MACameraOcclusionCutout::CameraPositionParameter,
		FLinearColor(CameraPosition.X, CameraPosition.Y, CameraPosition.Z, 1.f));
	Parameters->SetVectorParameterValue(
		MACameraOcclusionCutout::TargetPositionParameter,
		FLinearColor(TargetPosition.X, TargetPosition.Y, TargetPosition.Z, 1.f));
	Parameters->SetScalarParameterValue(MACameraOcclusionCutout::RadiusParameter, CutoutRadius);
	Parameters->SetScalarParameterValue(MACameraOcclusionCutout::EnabledParameter, 1.f);
}

void UMACameraOcclusionCutoutComponent::SetCutoutEnabled(const bool bEnabled) const
{
	UWorld* World = GetWorld();
	if (!World || !ParameterCollection) return;

	if (UMaterialParameterCollectionInstance* Parameters =
		World->GetParameterCollectionInstance(ParameterCollection))
	{
		Parameters->SetScalarParameterValue(
			MACameraOcclusionCutout::EnabledParameter,
			bEnabled ? 1.f : 0.f);
	}
}
