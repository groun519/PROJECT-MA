#include "Player/Cursor/MACursorSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Character/MACharacter.h"
#include "Convenience/MAHighlightComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/LocalPlayer.h"
#include "Engine/UserInterfaceSettings.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "P_MA/P_MA.h"
#include "TimerManager.h"
#include "Widget/Cursor/MACursorWidget.h"

void UMACursorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		CachedPlayerController = LP->GetPlayerController(GetWorld());
	}

	RestartCursorTimer();
}

void UMACursorSubsystem::Deinitialize()
{
	StopCursorTimer();
	ClearHoveredActorHighlight();
	CursorWidgetInstance = nullptr;
	CachedPlayerController = nullptr;

	Super::Deinitialize();
}

void UMACursorSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	StopCursorTimer();
	ClearHoveredActorHighlight();
	CachedPlayerController = NewPlayerController;
	CursorWidgetInstance = nullptr;
	CursorTargetRelation = ECursorTargetRelation::None;

	RestartCursorTimer();
}

void UMACursorSubsystem::RestartCursorTimer()
{
	StopCursorTimer();

	APlayerController* PC = CachedPlayerController.Get();
	if (!PC || !PC->IsLocalController()) return;

	InitializeRuntimeCursorWidget();
	RefreshCursorTargetRelation();

	UWorld* World = PC->GetWorld();
	if (!World) return;

	World->GetTimerManager().SetTimer(
		CursorRelationTimerHandle,
		this,
		&UMACursorSubsystem::RefreshCursorTargetRelation,
		CursorRelationCheckInterval,
		true,
		CursorRelationCheckInterval);
}

void UMACursorSubsystem::StopCursorTimer()
{
	UWorld* World = CachedPlayerController ? CachedPlayerController->GetWorld() : GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(CursorRelationTimerHandle);
}

bool UMACursorSubsystem::GetAimDirection(FVector& OutDirection) const
{
	APlayerController* PC = CachedPlayerController.Get();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn) return false;

	FVector TargetLocation;
	if (const AMACharacter* TargetCharacter = HighlightedActor.Get())
	{
		TargetLocation = TargetCharacter->GetActorLocation();
	}
	else
	{
		FHitResult Hit;
		if (!TraceVisibilityUnderCursor(Hit)) return false;
		TargetLocation = Hit.Location;
	}

	const FVector Origin = Pawn->GetActorLocation();
	TargetLocation.Z = Origin.Z;
	OutDirection = (TargetLocation - Origin).GetSafeNormal();
	return !OutDirection.IsNearlyZero();
}

void UMACursorSubsystem::RefreshCursorTargetRelation()
{
	AMACharacter* HoveredCharacter = ResolveHoveredCharacter();
	const ECursorTargetRelation NewRelation = ResolveCursorTargetRelation(HoveredCharacter);
	UpdateHoveredActorHighlight(HoveredCharacter, NewRelation);

	if (NewRelation == CursorTargetRelation) return;

	CursorTargetRelation = NewRelation;
	ApplyCursorRelationColor(CursorTargetRelation);
}

AMACharacter* UMACursorSubsystem::ResolveCharacterFromHit(const FHitResult& HitResult)
{
	for (AActor* Actor = HitResult.GetActor(); Actor; Actor = Actor->GetAttachParentActor())
	{
		if (AMACharacter* Character = Cast<AMACharacter>(Actor)) return Character;
	}

	return nullptr;
}

AMACharacter* UMACursorSubsystem::ResolveHoveredCharacter() const
{
	APlayerController* PC = CachedPlayerController.Get();
	if (!PC || !PC->IsLocalController()) return nullptr;

	static const TArray<TEnumAsByte<EObjectTypeQuery>> CursorTargetTypes =
	{
		UEngineTypes::ConvertToObjectType(ECC_Hitbox)
	};

	FHitResult TargetHit;
	if (!PC->GetHitResultUnderCursorForObjects(CursorTargetTypes, false, TargetHit)) return nullptr;

	AMACharacter* HitCharacter = ResolveCharacterFromHit(TargetHit);
	if (!HitCharacter) return nullptr;

	FHitResult VisibilityHit;
	if (TraceVisibilityUnderCursor(VisibilityHit)
		&& VisibilityHit.Distance < TargetHit.Distance
		&& ResolveCharacterFromHit(VisibilityHit) != HitCharacter)
	{
		return nullptr;
	}

	return HitCharacter;
}

bool UMACursorSubsystem::TraceVisibilityUnderCursor(FHitResult& OutHit) const
{
	APlayerController* PC = CachedPlayerController.Get();
	if (!PC) return false;

	FVector WorldOrigin;
	FVector WorldDirection;
	if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection)) return false;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PC->GetPawn());
	return PC->GetWorld()->LineTraceSingleByChannel(
		OutHit,
		WorldOrigin,
		WorldOrigin + WorldDirection * PC->HitResultTraceDistance,
		ECC_Visibility,
		QueryParams);
}

ECursorTargetRelation UMACursorSubsystem::ResolveCursorTargetRelation(AActor* HitActor) const
{
	APlayerController* PC = CachedPlayerController.Get();
	if (!PC || !PC->IsLocalController()) return ECursorTargetRelation::None;

	APawn* SelfPawn = PC->GetPawn();
	if (!SelfPawn) return ECursorTargetRelation::None;

	if (!HitActor)
	{
		return ECursorTargetRelation::None;
	}

	if (HitActor == SelfPawn)
	{
		return ECursorTargetRelation::Neutral;
	}

	const IGenericTeamAgentInterface* SelfTeamInterface = Cast<IGenericTeamAgentInterface>(SelfPawn);
	const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(HitActor);
	if (!SelfTeamInterface || !OtherTeamInterface)
	{
		return ECursorTargetRelation::Neutral;
	}

	switch (SelfTeamInterface->GetTeamAttitudeTowards(*HitActor))
	{
	case ETeamAttitude::Friendly:
		return ECursorTargetRelation::Friendly;
	case ETeamAttitude::Hostile:
		return ECursorTargetRelation::Hostile;
	default:
		return ECursorTargetRelation::Neutral;
	}
}

void UMACursorSubsystem::UpdateHoveredActorHighlight(
	AMACharacter* HoveredCharacter,
	ECursorTargetRelation InRelation)
{
	if (HighlightedActor.Get() == HoveredCharacter && HighlightedActorRelation == InRelation)
		return;

	ClearHoveredActorHighlight();
	if (!HoveredCharacter || InRelation == ECursorTargetRelation::None) return;

	UMAHighlightComponent* Highlighter = HoveredCharacter->GetHighlightComponent();
	if (!Highlighter) return;

	HighlightedActor = HoveredCharacter;
	HighlightedComponent = Highlighter;
	HighlightedActorRelation = InRelation;
	Highlighter->SetHighlight(
		*this,
		true,
		ResolveCursorRelationColor(InRelation));
}

void UMACursorSubsystem::ClearHoveredActorHighlight()
{
	if (UMAHighlightComponent* Highlighter = HighlightedComponent.Get())
	{
		Highlighter->SetHighlight(*this, false);
	}

	HighlightedComponent.Reset();
	HighlightedActor.Reset();
	HighlightedActorRelation = ECursorTargetRelation::None;
}

void UMACursorSubsystem::InitializeRuntimeCursorWidget()
{
	APlayerController* PC = CachedPlayerController.Get();
	if (!PC || !PC->IsLocalController()) return;
	if (CursorWidgetInstance) return;

	const TSubclassOf<UMACursorWidget> CursorClass = ResolveCursorWidgetClass();
	if (!CursorClass) return;

	CursorWidgetInstance = CreateWidget<UMACursorWidget>(PC, CursorClass);
	if (!CursorWidgetInstance) return;

	CursorWidgetInstance->SetIsFocusable(false);
	CursorWidgetInstance->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	PC->SetMouseCursorWidget(EMouseCursor::Default, CursorWidgetInstance);
	ApplyCursorRelationColor(CursorTargetRelation);
}

TSubclassOf<UMACursorWidget> UMACursorSubsystem::ResolveCursorWidgetClass()
{
	const UUserInterfaceSettings* UISettings = GetDefault<UUserInterfaceSettings>();
	if (!UISettings) return nullptr;

	const FSoftClassPath* CursorClassPath = UISettings->SoftwareCursors.Find(EMouseCursor::Default);
	if (!CursorClassPath) return nullptr;

	UClass* LoadedClass = CursorClassPath->TryLoadClass<UUserWidget>();
	if (!LoadedClass || !LoadedClass->IsChildOf(UMACursorWidget::StaticClass()))
	{
		return nullptr;
	}

	return LoadedClass;
}

void UMACursorSubsystem::ApplyCursorRelationColor(ECursorTargetRelation InRelation)
{
	if (!CursorWidgetInstance) return;

	CursorWidgetInstance->SetBaseColor(ResolveCursorRelationColor(InRelation));
}

FLinearColor UMACursorSubsystem::ResolveCursorRelationColor(ECursorTargetRelation InRelation) const
{
	FLinearColor TargetColor = CursorNeutralColor;
	switch (InRelation)
	{
	case ECursorTargetRelation::Friendly:
		TargetColor = CursorFriendlyColor;
		break;
	case ECursorTargetRelation::Hostile:
		TargetColor = CursorHostileColor;
		break;
	case ECursorTargetRelation::Neutral:
	case ECursorTargetRelation::None:
	default:
		TargetColor = CursorNeutralColor;
		break;
	}

	return TargetColor;
}
