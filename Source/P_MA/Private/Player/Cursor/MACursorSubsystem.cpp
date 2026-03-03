// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Cursor/MACursorSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/UserInterfaceSettings.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
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
	CursorWidgetInstance = nullptr;
	CachedPlayerController = nullptr;

	Super::Deinitialize();
}

void UMACursorSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	StopCursorTimer();
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
	const ECursorTargetRelation NewRelation = ResolveCursorTargetRelation();
	if (NewRelation == CursorTargetRelation) return;

	CursorTargetRelation = NewRelation;
	ApplyCursorRelationColor(CursorTargetRelation);
	OnCursorTargetRelationChanged.Broadcast(CursorTargetRelation);
}

ECursorTargetRelation UMACursorSubsystem::ResolveCursorTargetRelation()
{
	APlayerController* PC = CachedPlayerController.Get();
	if (!PC || !PC->IsLocalController()) return ECursorTargetRelation::None;

	APawn* SelfPawn = PC->GetPawn();
	if (!SelfPawn) return ECursorTargetRelation::None;

	FHitResult MouseHitResult;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, MouseHitResult);

	AActor* HitActor = MouseHitResult.GetActor();
	if (!MouseHitResult.bBlockingHit || !HitActor)
	{
		return ECursorTargetRelation::None;
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
