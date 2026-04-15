#include "Player/Cursor/MACursorSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Character/MACharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/UserInterfaceSettings.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "Player/MAPlayerCharacter.h"
#include "TimerManager.h"
#include "Weapon/WeaponComponent.h"
#include "Widget/Cursor/MACursorWidget.h"

namespace
{
AMACharacter* ResolveHoveredCharacter(const FHitResult& HitResult)
{
	TArray<AActor*> CandidateActors;
	if (AActor* HitActor = HitResult.GetActor())
	{
		CandidateActors.Add(HitActor);
	}

	if (UPrimitiveComponent* HitComponent = HitResult.GetComponent())
	{
		if (AActor* ComponentOwner = HitComponent->GetOwner())
		{
			CandidateActors.AddUnique(ComponentOwner);
		}

		if (AActor* AttachmentRootActor = HitComponent->GetAttachmentRootActor())
		{
			CandidateActors.AddUnique(AttachmentRootActor);
		}
	}

	for (AActor* CandidateActor : CandidateActors)
	{
		for (AActor* CurrentActor = CandidateActor; CurrentActor; CurrentActor = CurrentActor->GetAttachParentActor())
		{
			if (AMACharacter* HitCharacter = Cast<AMACharacter>(CurrentActor))
			{
				return HitCharacter;
			}
		}
	}

	return nullptr;
}
}

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

void UMACursorSubsystem::RefreshCursorTargetRelation()
{
	const FHoveredCursorTarget HoveredTarget = ResolveHoveredTarget();
	const ECursorTargetRelation NewRelation = ResolveCursorTargetRelation(HoveredTarget.Actor.Get());
	UpdateHoveredActorHighlight(HoveredTarget, NewRelation);

	if (NewRelation == CursorTargetRelation) return;

	CursorTargetRelation = NewRelation;
	ApplyCursorRelationColor(CursorTargetRelation);
}

UMACursorSubsystem::FHoveredCursorTarget UMACursorSubsystem::ResolveHoveredTarget() const
{
	APlayerController* PC = CachedPlayerController.Get();
	if (!PC || !PC->IsLocalController()) return {};

	FHitResult MouseHitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, MouseHitResult);
	if (!MouseHitResult.bBlockingHit) return {};

	AMACharacter* HitCharacter = ResolveHoveredCharacter(MouseHitResult);
	if (!HitCharacter) return {};

	FHoveredCursorTarget Result;
	Result.Actor = HitCharacter;
	return Result;
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

void UMACursorSubsystem::UpdateHoveredActorHighlight(const FHoveredCursorTarget& HoveredTarget, ECursorTargetRelation InRelation)
{
	if (HighlightedActor.Get() == HoveredTarget.Actor.Get() && HighlightedActorRelation == InRelation)
		return;

	ClearHoveredActorHighlight();
	if (!HoveredTarget.Actor.IsValid() || InRelation == ECursorTargetRelation::None)
		return;

	int32 StencilValue;
	switch (InRelation)
	{
	case ECursorTargetRelation::Friendly:
		StencilValue = FriendlyHighlightStencil;
		break;
	case ECursorTargetRelation::Hostile:
		StencilValue = HostileHighlightStencil;
		break;
	case ECursorTargetRelation::Neutral:
	default:
		StencilValue = NeutralHighlightStencil;
		break;
	}

	HighlightedActor = HoveredTarget.Actor;
	HighlightedActorRelation = InRelation;

	AMACharacter* HitCharacter = Cast<AMACharacter>(HoveredTarget.Actor.Get());
	if (!HitCharacter) return;

	TArray<UPrimitiveComponent*, TInlineAllocator<2>> HighlightComponents;
	if (UPrimitiveComponent* MeshComponent = HitCharacter->GetMesh())
	{
		HighlightComponents.Add(MeshComponent);
	}

	if (AMAPlayerCharacter* HitPlayerCharacter = Cast<AMAPlayerCharacter>(HitCharacter))
	{
		if (UWeaponComponent* WeaponComponent = HitPlayerCharacter->GetWeaponComponent())
		{
			HighlightComponents.AddUnique(WeaponComponent);
		}
	}

	for (UPrimitiveComponent* PrimitiveComponent : HighlightComponents)
	{
		if (!PrimitiveComponent || !PrimitiveComponent->IsRegistered()) continue;

		FHighlightedPrimitiveState& State = HighlightedPrimitiveStates.AddDefaulted_GetRef();
		State.Component = PrimitiveComponent;
		State.bPreviousRenderCustomDepth = PrimitiveComponent->bRenderCustomDepth;
		State.PreviousCustomDepthStencilValue = PrimitiveComponent->CustomDepthStencilValue;

		PrimitiveComponent->SetRenderCustomDepth(true);
		PrimitiveComponent->SetCustomDepthStencilValue(StencilValue);
	}
}

void UMACursorSubsystem::ClearHoveredActorHighlight()
{
	for (const FHighlightedPrimitiveState& State : HighlightedPrimitiveStates)
	{
		UPrimitiveComponent* PrimitiveComponent = State.Component.Get();
		if (!PrimitiveComponent) continue;

		PrimitiveComponent->SetRenderCustomDepth(State.bPreviousRenderCustomDepth);
		PrimitiveComponent->SetCustomDepthStencilValue(State.PreviousCustomDepthStencilValue);
	}

	HighlightedPrimitiveStates.Reset();
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

	CursorWidgetInstance->SetBaseColor(TargetColor);
}
