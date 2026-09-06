#include "Player/Spectate/MAPlayerSpectateComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Input/MAInputStatics.h"
#include "InputMappingContext.h"
#include "Player/Camera/MACameraLibrary.h"
#include "Player/Camera/MACameraOcclusionCutoutComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerControllerBase.h"
#include "Widget/Spectate/MASpectateOverlayWidget.h"

UMAPlayerSpectateComponent::UMAPlayerSpectateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMAPlayerSpectateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearBoundPawnDeathBinding();
	BoundPlayerCharacter.Reset();
	ClearSpectateTargetBinding();
	SetSpectateInputMappingEnabled(false);
	RemoveSpectateOverlay();
	Super::EndPlay(EndPlayReason);
}

void UMAPlayerSpectateComponent::BindToPawn(AMAPlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter && BoundPlayerCharacter.Get() == PlayerCharacter) return;

	ClearBoundPawnDeathBinding();
	BoundPlayerCharacter = PlayerCharacter;
	if (!PlayerCharacter) return;

	if (!GetLocalOwnerPlayerController()) return;

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	{
		DeadTagChangedHandle = ASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetDeadStatTag())
			.AddUObject(this, &UMAPlayerSpectateComponent::HandleDeadTagChanged);

		const int32 CurrentDeadCount = ASC->GetTagCount(UMAAbilitySystemStatics::GetDeadStatTag());
		HandleDeadTagChanged(UMAAbilitySystemStatics::GetDeadStatTag(), CurrentDeadCount);
	}
}

void UMAPlayerSpectateComponent::RequestPawnCamera(float BlendTime)
{
	APlayerController* PlayerController = GetLocalOwnerPlayerController();
	if (!PlayerController) return;

	SetSpectating(false);
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		const float CameraBlendTime = BlendTime >= 0.f ? BlendTime : DeathSpectateBlendTime;
		ApplyCameraTarget(*Pawn, CameraBlendTime);
	}
}

void UMAPlayerSpectateComponent::HandleDeadTagChanged(const FGameplayTag /*Tag*/, int32 NewCount)
{
	if (NewCount > 0)
	{
		SetSpectating(true);
		RefreshSpectateTargets(BoundPlayerCharacter.Get());
		ApplySpectateIndex(SpectateTargets.IsValidIndex(0) ? 0 : INDEX_NONE);
		return;
	}

	if (bSpectating)
	{
		RequestPawnCamera();
	}
}

void UMAPlayerSpectateComponent::HandleSpectateTargetDeadTagChanged(const FGameplayTag /*Tag*/, int32 NewCount)
{
	if (!bSpectating || NewCount <= 0) return;

	SelectNextSpectateTarget(ObservedSpectateTarget.Get());
}

void UMAPlayerSpectateComponent::HandleSpectateTargetDestroyed(AActor* DestroyedActor)
{
	if (!bSpectating) return;

	SelectNextSpectateTarget(DestroyedActor);
}

void UMAPlayerSpectateComponent::SpectateRight()
{
	if (!bSpectating) return;

	RefreshSpectateTargets();
	if (SpectateTargets.Num() <= 1)
	{
		ApplySpectateIndex(SpectateTargets.IsValidIndex(0) ? 0 : INDEX_NONE);
		return;
	}

	ApplySpectateIndex((CurrentSpectateIndex + 1) % SpectateTargets.Num());
}

void UMAPlayerSpectateComponent::SpectateLeft()
{
	if (!bSpectating) return;

	RefreshSpectateTargets();
	if (SpectateTargets.Num() <= 1)
	{
		ApplySpectateIndex(SpectateTargets.IsValidIndex(0) ? 0 : INDEX_NONE);
		return;
	}

	const int32 PreviousIndex = (CurrentSpectateIndex - 1 + SpectateTargets.Num()) % SpectateTargets.Num();
	ApplySpectateIndex(PreviousIndex);
}

void UMAPlayerSpectateComponent::BindInput(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!EnhancedInputComponent) return;

	if (SpectateLeftInputAction)
	{
		EnhancedInputComponent->BindAction(
			SpectateLeftInputAction,
			ETriggerEvent::Started,
			this,
			&UMAPlayerSpectateComponent::SpectateLeft);
	}
	if (SpectateRightInputAction)
	{
		EnhancedInputComponent->BindAction(
			SpectateRightInputAction,
			ETriggerEvent::Started,
			this,
			&UMAPlayerSpectateComponent::SpectateRight);
	}
}

void UMAPlayerSpectateComponent::StopSpectating()
{
	SetSpectating(false);
}

AMAPlayerCharacter* UMAPlayerSpectateComponent::GetCurrentSpectateTarget() const
{
	return SpectateTargets.IsValidIndex(CurrentSpectateIndex)
		? SpectateTargets[CurrentSpectateIndex].Get()
		: nullptr;
}

void UMAPlayerSpectateComponent::SetSpectating(bool bNewSpectating)
{
	if (bSpectating == bNewSpectating) return;

	bSpectating = bNewSpectating;
	if (!bSpectating)
	{
		ClearSpectateTargetBinding();
		SpectateTargets.Reset();
		CurrentSpectateIndex = INDEX_NONE;
	}

	SetSpectateInputMappingEnabled(bSpectating);
	if (bSpectating)
	{
		ShowSpectateOverlay();
		return;
	}

	RemoveSpectateOverlay();
}

void UMAPlayerSpectateComponent::SelectNextSpectateTarget(AActor* LostTarget)
{
	RefreshSpectateTargets(LostTarget);
	ApplySpectateIndex(SpectateTargets.IsValidIndex(0) ? 0 : INDEX_NONE);
}

void UMAPlayerSpectateComponent::ApplySpectateIndex(int32 NewIndex)
{
	CurrentSpectateIndex = SpectateTargets.IsValidIndex(NewIndex) ? NewIndex : INDEX_NONE;

	AMAPlayerCharacter* SpectateTarget = GetCurrentSpectateTarget();
	BindSpectateTarget(SpectateTarget);
	if (SpectateTarget)
	{
		ApplyCameraTarget(*SpectateTarget, DeathSpectateBlendTime);
	}

	if (SpectateOverlayWidget)
	{
		SpectateOverlayWidget->SetSpectateTarget(SpectateTarget, SpectateTargets.Num());
	}
}

void UMAPlayerSpectateComponent::ApplyCameraTarget(AActor& Target, const float BlendTime)
{
	APlayerController* PlayerController = GetLocalOwnerPlayerController();
	if (!PlayerController) return;

	FMACameraLibrary::SwitchViewTarget(*PlayerController, Target, BlendTime);
	if (AMAPlayerControllerBase* MAPlayerController = Cast<AMAPlayerControllerBase>(PlayerController))
	{
		if (UMACameraOcclusionCutoutComponent* OcclusionCutout = MAPlayerController->GetCameraOcclusionCutout())
		{
			OcclusionCutout->RevealTarget(*PlayerController, Target);
		}
	}
}

void UMAPlayerSpectateComponent::SetSpectateInputMappingEnabled(bool bEnabled)
{
	APlayerController* PlayerController = GetLocalOwnerPlayerController();
	if (!PlayerController || !SpectateInputMapping) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController->GetLocalPlayer()
		? PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()
		: nullptr;
	if (!InputSubsystem) return;

	InputSubsystem->RemoveMappingContext(SpectateInputMapping);
	if (bEnabled)
	{
		FMAInputStatics::RegisterInputMappingContextDefaults(PlayerController, SpectateInputMapping);
		InputSubsystem->AddMappingContext(SpectateInputMapping, SpectateInputMappingPriority);
	}
}

void UMAPlayerSpectateComponent::RefreshSpectateTargets(AActor* DeadActor)
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector Origin = DeadActor ? DeadActor->GetActorLocation() : FVector::ZeroVector;
	AMAPlayerCharacter* PreviousTarget = GetCurrentSpectateTarget();

	TArray<TPair<float, TWeakObjectPtr<AMAPlayerCharacter>>> SortedTargets;

	for (TActorIterator<AMAPlayerCharacter> It(World); It; ++It)
	{
		AMAPlayerCharacter* Candidate = *It;
		if (!Candidate || Candidate == DeadActor || Candidate->IsDead()) continue;

		const float DistanceSquared = DeadActor
			? FVector::DistSquared(Origin, Candidate->GetActorLocation())
			: 0.f;
		SortedTargets.Emplace(DistanceSquared, Candidate);
	}

	SortedTargets.Sort([](const TPair<float, TWeakObjectPtr<AMAPlayerCharacter>>& A,
		const TPair<float, TWeakObjectPtr<AMAPlayerCharacter>>& B)
	{
		return A.Key < B.Key;
	});

	SpectateTargets.Reset(SortedTargets.Num());
	CurrentSpectateIndex = INDEX_NONE;
	for (const TPair<float, TWeakObjectPtr<AMAPlayerCharacter>>& Entry : SortedTargets)
	{
		const int32 NewIndex = SpectateTargets.Add(Entry.Value);
		if (Entry.Value.Get() == PreviousTarget)
		{
			CurrentSpectateIndex = NewIndex;
		}
	}

	if (CurrentSpectateIndex == INDEX_NONE && SpectateTargets.Num() > 0)
	{
		CurrentSpectateIndex = 0;
	}
}

void UMAPlayerSpectateComponent::ClearBoundPawnDeathBinding()
{
	if (UAbilitySystemComponent* ASC = BoundPlayerCharacter.IsValid()
		? BoundPlayerCharacter->GetAbilitySystemComponent()
		: nullptr)
	{
		if (DeadTagChangedHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetDeadStatTag()).Remove(DeadTagChangedHandle);
		}
	}

	DeadTagChangedHandle.Reset();
}

void UMAPlayerSpectateComponent::BindSpectateTarget(AMAPlayerCharacter* SpectateTarget)
{
	if (SpectateTarget && ObservedSpectateTarget.Get() == SpectateTarget && SpectateTargetDeadTagChangedHandle.IsValid()) return;

	ClearSpectateTargetBinding();
	ObservedSpectateTarget = SpectateTarget;
	if (!SpectateTarget) return;

	SpectateTarget->OnDestroyed.AddDynamic(this, &UMAPlayerSpectateComponent::HandleSpectateTargetDestroyed);

	if (UAbilitySystemComponent* ASC = SpectateTarget->GetAbilitySystemComponent())
	{
		SpectateTargetDeadTagChangedHandle = ASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetDeadStatTag())
			.AddUObject(this, &UMAPlayerSpectateComponent::HandleSpectateTargetDeadTagChanged);

		const int32 CurrentDeadCount = ASC->GetTagCount(UMAAbilitySystemStatics::GetDeadStatTag());
		HandleSpectateTargetDeadTagChanged(UMAAbilitySystemStatics::GetDeadStatTag(), CurrentDeadCount);
	}
}

void UMAPlayerSpectateComponent::ClearSpectateTargetBinding()
{
	if (AMAPlayerCharacter* SpectateTarget = ObservedSpectateTarget.Get())
	{
		SpectateTarget->OnDestroyed.RemoveDynamic(this, &UMAPlayerSpectateComponent::HandleSpectateTargetDestroyed);

		if (UAbilitySystemComponent* ASC = SpectateTarget->GetAbilitySystemComponent())
		{
			if (SpectateTargetDeadTagChangedHandle.IsValid())
			{
				ASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetDeadStatTag()).Remove(SpectateTargetDeadTagChangedHandle);
			}
		}
	}

	SpectateTargetDeadTagChangedHandle.Reset();
	ObservedSpectateTarget.Reset();
}

void UMAPlayerSpectateComponent::ShowSpectateOverlay()
{
	APlayerController* PlayerController = GetLocalOwnerPlayerController();
	if (!PlayerController || SpectateOverlayWidget || !SpectateOverlayWidgetClass) return;

	SpectateOverlayWidget = CreateWidget<UMASpectateOverlayWidget>(PlayerController, SpectateOverlayWidgetClass);
	if (!SpectateOverlayWidget) return;

	SpectateOverlayWidget->AddToViewport();
	SpectateOverlayWidget->InitializeSpectateOverlay(PlayerController, SpectateInputMapping);
	SpectateOverlayWidget->SetSpectateTarget(GetCurrentSpectateTarget(), SpectateTargets.Num());
}

void UMAPlayerSpectateComponent::RemoveSpectateOverlay()
{
	if (!SpectateOverlayWidget) return;

	SpectateOverlayWidget->RemoveFromParent();
	SpectateOverlayWidget = nullptr;
}
