#include "Level/Transition/MASpaceTransitionMask.h"

#include "MAMaterialParams.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Level/Transition/MASpaceTransitionVisibilityComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"
#include "UObject/UObjectIterator.h"

UWorld* UMASpaceTransitionMask::GetWorld() const
{
	return GetTypedOuter<UWorld>();
}

bool UMASpaceTransitionMask::Close(const FVector& Center)
{
	if (!CreateMask()) return false;

	SetCenter(Center);
	SetProgress(1.f);
	CollectVisibleSubjects();
	return true;
}

bool UMASpaceTransitionMask::Open(const FVector& Center)
{
	if (!TransitionMaterialInstance) return false;

	SetCenter(Center);
	return true;
}

void UMASpaceTransitionMask::SetProgress(const float Progress) const
{
	TransitionMaterialInstance->SetScalarParameterValue(
		PARAM_SpaceTransition_Radius,
		OpenRadius * Progress);
}

void UMASpaceTransitionMask::Reset()
{
	ReleaseMask();
}

bool UMASpaceTransitionMask::CreateMask()
{
	UWorld* World = GetWorld();
	UMaterialInterface* TransitionMaterial = UMAGameSettings::Get()->GetSpaceTransitionMaterial();
	if (!ensureMsgf(TransitionMaterial, TEXT("Space Transition material is not configured."))) return false;

	TransitionMaterialInstance = UMaterialInstanceDynamic::Create(TransitionMaterial, this);
	if (!TransitionMaterialInstance) return false;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags |= RF_Transient;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APostProcessVolume* Volume = World->SpawnActor<APostProcessVolume>(
		APostProcessVolume::StaticClass(),
		FTransform::Identity,
		SpawnParameters);
	if (!Volume)
	{
		TransitionMaterialInstance = nullptr;
		return false;
	}

	Volume->bUnbound = true;
	Volume->Priority = TNumericLimits<float>::Max();
	Volume->BlendWeight = 1.f;
	Volume->AddOrUpdateBlendable(TransitionMaterialInstance, 1.f);
	TransitionVolume = Volume;
	return true;
}

void UMASpaceTransitionMask::SetCenter(const FVector& Center) const
{
	TransitionMaterialInstance->SetVectorParameterValue(
		PARAM_SpaceTransition_Center,
		FLinearColor(Center.X, Center.Y, Center.Z, 1.f));
}

void UMASpaceTransitionMask::CollectVisibleSubjects()
{
	for (TObjectIterator<UMASpaceTransitionVisibilityComponent> It; It; ++It)
	{
		if (It->GetWorld() != GetWorld()) continue;

		ActiveVisibleSubjects.Add(*It);
		It->SetVisibleThroughTransition(true);
	}
}

void UMASpaceTransitionMask::ReleaseMask()
{
	for (const TWeakObjectPtr<UMASpaceTransitionVisibilityComponent>& Subject : ActiveVisibleSubjects)
	{
		if (UMASpaceTransitionVisibilityComponent* Component = Subject.Get())
		{
			Component->SetVisibleThroughTransition(false);
		}
	}
	ActiveVisibleSubjects.Reset();

	if (APostProcessVolume* Volume = TransitionVolume.Get()) Volume->Destroy();
	TransitionVolume.Reset();
	TransitionMaterialInstance = nullptr;
}
