#include "Level/Transition/MASpaceTransitionMask.h"

#include "MAMaterialParams.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Level/Transition/MASpaceTransitionVisibilityComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"
#include "UObject/UObjectIterator.h"

UMASpaceTransitionMask::UMASpaceTransitionMask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FTickableGameObject(ETickableTickType::Never)
{
}

UWorld* UMASpaceTransitionMask::GetWorld() const
{
	return GetTypedOuter<UWorld>();
}

void UMASpaceTransitionMask::BeginDestroy()
{
	SetTickableTickType(ETickableTickType::Never);
	Super::BeginDestroy();
}

bool UMASpaceTransitionMask::Close(const FVector& Center, FSimpleDelegate OnClosed)
{
	if (Phase != EPhase::Open || !CreateMask()) return false;

	Phase = EPhase::Closing;
	TransitionFinishedDelegate = MoveTemp(OnClosed);
	TransitionMaterialInstance->SetVectorParameterValue(
		PARAM_SpaceTransition_Center,
		FLinearColor(Center.X, Center.Y, Center.Z, 1.f));
	UpdateRadius();
	CollectVisibleSubjects();
	SetTickableTickType(ETickableTickType::Always);
	return true;
}

bool UMASpaceTransitionMask::Open(const FVector& Center, FSimpleDelegate OnOpened)
{
	if (Phase != EPhase::Closed) return false;

	Phase = EPhase::Opening;
	TransitionFinishedDelegate = MoveTemp(OnOpened);
	TransitionMaterialInstance->SetVectorParameterValue(
		PARAM_SpaceTransition_Center,
		FLinearColor(Center.X, Center.Y, Center.Z, 1.f));
	SetTickableTickType(ETickableTickType::Always);
	return true;
}

void UMASpaceTransitionMask::Reset()
{
	SetTickableTickType(ETickableTickType::Never);
	TransitionFinishedDelegate.Unbind();
	Phase = EPhase::Open;
	TransitionAlpha = 1.f;
	ReleaseMask();
}

void UMASpaceTransitionMask::Tick(const float DeltaTime)
{
	const float Direction = Phase == EPhase::Closing ? -1.f : 1.f;
	TransitionAlpha = FMath::Clamp(
		TransitionAlpha + Direction * DeltaTime / TransitionDuration,
		0.f,
		1.f);
	UpdateRadius();

	if (TransitionAlpha > 0.f && TransitionAlpha < 1.f) return;

	SetTickableTickType(ETickableTickType::Never);
	FSimpleDelegate FinishedDelegate = TransitionFinishedDelegate;
	TransitionFinishedDelegate.Unbind();
	if (Phase == EPhase::Closing)
	{
		Phase = EPhase::Closed;
	}
	else
	{
		Phase = EPhase::Open;
		ReleaseMask();
	}
	FinishedDelegate.ExecuteIfBound();
}

TStatId UMASpaceTransitionMask::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMASpaceTransitionMask, STATGROUP_Tickables);
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

void UMASpaceTransitionMask::UpdateRadius() const
{
	TransitionMaterialInstance->SetScalarParameterValue(
		PARAM_SpaceTransition_Radius,
		FMath::InterpEaseInOut(0.f, OpenRadius, TransitionAlpha, 2.f));
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
