#include "Player/Components/ReadyCheckWidgetComponent.h"

#include "Framework/MAGameState.h"
#include "Player/Components/ReadyStateComponent.h"
#include "Widget/Loop/ReadyCheckWidget.h"
#include "TimerManager.h"

UReadyCheckWidgetComponent::UReadyCheckWidgetComponent()
{
}

void UReadyCheckWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	InitWidget();
	BindReadyStateDelegates();
	BindSectorStateDelegate();
	RequestReadyCheckPresentation();
}

void UReadyCheckWidgetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedReadyStateComponent)
	{
		CachedReadyStateComponent->OnReadyStateChanged.RemoveAll(this);
		CachedReadyStateComponent->OnLoopReadyStateChanged.RemoveAll(this);
	}

	if (CachedGameState.IsValid())
	{
		CachedGameState->OnMASectorStateChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UReadyCheckWidgetComponent::BindReadyStateDelegates()
{
	CachedReadyStateComponent = GetOwner() ? GetOwner()->FindComponentByClass<UReadyStateComponent>() : nullptr;
	if (!CachedReadyStateComponent) return;

	CachedReadyStateComponent->OnReadyStateChanged.RemoveAll(this);
	CachedReadyStateComponent->OnLoopReadyStateChanged.RemoveAll(this);
	CachedReadyStateComponent->OnReadyStateChanged.AddUObject(this, &UReadyCheckWidgetComponent::HandleReadyStateChanged);
	CachedReadyStateComponent->OnLoopReadyStateChanged.AddUObject(this, &UReadyCheckWidgetComponent::HandleReadyStateChanged);
}

void UReadyCheckWidgetComponent::BindSectorStateDelegate()
{
	AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr;
	if (!GS)
	{
		SetHiddenInGame(true);
		return;
	}

	CachedGameState = GS;
	CachedGameState->OnMASectorStateChanged.RemoveAll(this);
	CachedGameState->OnMASectorStateChanged.AddUObject(this, &UReadyCheckWidgetComponent::HandleSectorStateChanged);
	HandleSectorStateChanged(CachedGameState->GetMASectorState());
}

void UReadyCheckWidgetComponent::HandleReadyStateChanged(bool bIsReady)
{
	(void)bIsReady;
	RequestReadyCheckPresentation();
}

void UReadyCheckWidgetComponent::HandleSectorStateChanged(EMASectorState NewState)
{
	(void)NewState;
	RequestReadyCheckPresentation();
}

void UReadyCheckWidgetComponent::RequestReadyCheckPresentation()
{
	if (bReadyCheckPresentationPending) return;

	bReadyCheckPresentationPending = true;
	UWorld* World = GetWorld();
	if (!World)
	{
		FlushReadyCheckPresentation();
		return;
	}

	World->GetTimerManager().SetTimerForNextTick(this, &UReadyCheckWidgetComponent::FlushReadyCheckPresentation);
}

void UReadyCheckWidgetComponent::FlushReadyCheckPresentation()
{
	bReadyCheckPresentationPending = false;
	ApplyReadyCheckPresentation(ResolveReadyCheckSectorState());
}

void UReadyCheckWidgetComponent::ApplyReadyCheckPresentation(EMASectorState InState)
{
	if (!CachedReadyStateComponent) return;

	const bool bShouldShow =
		InState == EMASectorState::Wait ||
		InState == EMASectorState::EndBattle ||
		InState == EMASectorState::Loop;

	SetHiddenInGame(!bShouldShow);
	if (!bShouldShow) return;

	const bool bUseLoopReady = (InState == EMASectorState::Loop);
	const bool bReady = bUseLoopReady ? CachedReadyStateComponent->IsLoopReady() : CachedReadyStateComponent->IsReady();

	// Widget can be rebuilt while visibility/state changes, ensure we always target the current instance.
	if (!GetUserWidgetObject())
	{
		InitWidget();
	}

	if (UReadyCheckWidget* ReadyWidget = Cast<UReadyCheckWidget>(GetUserWidgetObject()))
	{
		ReadyWidget->SetReadyState(bReady);
	}
}

EMASectorState UReadyCheckWidgetComponent::ResolveReadyCheckSectorState() const
{
	if (const AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr)
	{
		return GS->GetMASectorState();
	}
	return EMASectorState::Wait;
}
