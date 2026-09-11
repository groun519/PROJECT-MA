#include "Level/Transition/MASpaceTransitionSubsystem.h"

#include "Audio/Gameplay/MAGameplaySoundSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Level/Environment/MASpaceDirectionalLight.h"
#include "Level/Streaming/MALevelRoot.h"
#include "Level/Streaming/MAStreamingLevelLoader.h"
#include "Level/Transition/MAMagicCircle.h"
#include "Level/Transition/MASpaceTransitionMask.h"
#include "Misc/Guid.h"
#include "Player/Camera/MACameraComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerControllerBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogMASpaceTransition, Log, All);

bool UMASpaceTransitionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMASpaceTransitionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LevelLoader = NewObject<UMAStreamingLevelLoader>(this);
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		TransitionMask = NewObject<UMASpaceTransitionMask>(this);
	}
}

void UMASpaceTransitionSubsystem::Deinitialize()
{
	if (TransitionMask) TransitionMask->Reset();
	TransitionMask = nullptr;
	LevelLoader->CancelPendingLoad();
	LevelLoader = nullptr;
	Super::Deinitialize();
}

void UMASpaceTransitionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	CurrentLevel = LevelLoader->RegisterInitialLevel();
	if (!ensureMsgf(CurrentLevel.IsValid(), TEXT("Space Transition requires an initial LevelRoot."))) return;
	if (!ensureMsgf(
		CurrentLevel->GetTransitionCircle() && CurrentLevel->GetDirectionalLight(),
		TEXT("The initial LevelRoot requires an assigned Transition Circle and Directional Light.")))
	{
		return;
	}
	if (TransitionMask) CurrentLevel->GetDirectionalLight()->ActivateLighting();
}

bool UMASpaceTransitionSubsystem::IsTickable() const
{
	return TransitionMask &&
		((Phase == EPhase::Closing && TransitionAlpha > 0.f) ||
		 (Phase == EPhase::Opening && TransitionAlpha < 1.f));
}

void UMASpaceTransitionSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const float Direction = Phase == EPhase::Closing ? -1.f : 1.f;
	TransitionAlpha = FMath::Clamp(
		TransitionAlpha + Direction * DeltaTime / TransitionDuration,
		0.f,
		1.f);
	const float Progress = FMath::InterpEaseInOut(0.f, 1.f, TransitionAlpha, 2.f);
	TransitionMask->SetProgress(Progress);

	if (Phase == EPhase::Opening)
	{
		CurrentLevel->GetDirectionalLight()->TransitionTo(
			Progress,
			DestinationLevel->GetDirectionalLight());
	}

	if (TransitionAlpha > 0.f && TransitionAlpha < 1.f) return;
	if (Phase == EPhase::Closing)
	{
		NotifyServer(true);
	}
	else
	{
		CompleteLocalOpen(true);
	}
}

TStatId UMASpaceTransitionSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMASpaceTransitionSubsystem, STATGROUP_Tickables);
}

bool UMASpaceTransitionSubsystem::RequestTransition(
	TSoftObjectPtr<UWorld> DestinationMap,
	const FTransform& DestinationSlotTransform,
	const int32 GenerationSeed)
{
	UWorld* World = GetWorld();
	if (World->GetNetMode() == NM_Client || Phase != EPhase::Idle) return false;
	if (!ensureMsgf(
		CurrentLevel.IsValid() && CurrentLevel->GetTransitionCircle() && CurrentLevel->GetDirectionalLight(),
		TEXT("Space Transition requires a Current LevelRoot with an assigned Transition Circle and Directional Light.")))
	{
		return false;
	}
	if (!ensureMsgf(!DestinationMap.IsNull(), TEXT("Space Transition requires a Destination map."))) return false;

	ActiveRequest.DestinationMap = DestinationMap;
	ActiveRequest.DestinationSlotTransform = DestinationSlotTransform;
	ActiveRequest.DestinationInstanceIdentity = FString::Printf(
		TEXT("MA_Level_%s"),
		*FGuid::NewGuid().ToString(EGuidFormats::Digits));
	ActiveRequest.GenerationSeed = GenerationSeed;
	Phase = EPhase::Loading;
	UE_LOG(LogMASpaceTransition, Log, TEXT("Loading Space '%s' as '%s'."),
		*DestinationMap.ToString(), *ActiveRequest.DestinationInstanceIdentity);

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(It->Get());
		if (!ensureMsgf(PlayerController, TEXT("Space Transition requires MAPlayerControllerBase.")))
		{
			ResetTransitionState();
			return false;
		}

		if (!PlayerController->IsLocalController()) PendingPlayers.Add(PlayerController);
	}

	if (!LoadDestination(ActiveRequest))
	{
		ResetTransitionState();
		return false;
	}

	for (const TWeakObjectPtr<AMAPlayerControllerBase>& PendingPlayer : PendingPlayers)
	{
		if (AMAPlayerControllerBase* PlayerController = PendingPlayer.Get())
		{
			PlayerController->ClientPrepareSpaceTransition(ActiveRequest);
		}
	}

	return true;
}

void UMASpaceTransitionSubsystem::BeginClientPrepare(const FMASpaceTransitionRequest& Request)
{
	if (!Request.IsValid() || Phase != EPhase::Idle) return;

	ActiveRequest = Request;
	Phase = EPhase::Loading;
	if (!LoadDestination(Request))
	{
		NotifyServer(false);
	}
}

void UMASpaceTransitionSubsystem::BeginLocalClose(AMAPlayerCharacter* PlayerCharacter)
{
	const EPhase ExpectedPhase =
		GetWorld()->GetNetMode() == NM_Client ? EPhase::Loading : EPhase::Closing;
	if (!ActiveRequest.IsValid() || Phase != ExpectedPhase ||
		!CurrentLevel.IsValid() || !DestinationLevel.IsValid())
	{
		NotifyServer(false);
		return;
	}

	AMAMagicCircle* SourceCircle = CurrentLevel->GetTransitionCircle();
	if (!SourceCircle || !TransitionMask->Close(SourceCircle->GetActorLocation()))
	{
		TransitionAlpha = 0.f;
		NotifyServer(false);
		return;
	}
	Phase = EPhase::Closing;
	TransitionAlpha = 1.f;
	LocalPlayerCharacter = PlayerCharacter;
	if (PlayerCharacter) PlayerCharacter->GetPlayerCamera()->SetLocationLagEnabled(false);

	static const FGameplayTag CloseSoundTag =
		FGameplayTag::RequestGameplayTag(TEXT("Sound.SpaceTransition.Close"));
	PlayTransitionSound(CloseSoundTag, *SourceCircle);
}

void UMASpaceTransitionSubsystem::BeginLocalOpen()
{
	const EPhase ExpectedPhase =
		GetWorld()->GetNetMode() == NM_Client ? EPhase::Closing : EPhase::Opening;
	if (!ActiveRequest.IsValid() || Phase != ExpectedPhase ||
		!CurrentLevel.IsValid() || !DestinationLevel.IsValid())
	{
		NotifyServer(false);
		return;
	}

	AMAMagicCircle* DestinationCircle = DestinationLevel->GetTransitionCircle();
	if (!DestinationCircle || !TransitionMask->Open(DestinationCircle->GetActorLocation()))
	{
		TransitionAlpha = 1.f;
		CompleteLocalOpen(false);
		return;
	}

	Phase = EPhase::Opening;
	TransitionAlpha = 0.f;

	static const FGameplayTag OpenSoundTag =
		FGameplayTag::RequestGameplayTag(TEXT("Sound.SpaceTransition.Open"));
	PlayTransitionSound(OpenSoundTag, *DestinationCircle);
}

void UMASpaceTransitionSubsystem::AbortLocalTransition()
{
	if (GetWorld()->GetNetMode() != NM_Client || !ActiveRequest.IsValid() ||
		(Phase != EPhase::Loading && Phase != EPhase::Closing))
	{
		return;
	}

	TransitionMask->Reset();
	DiscardDestination();
	ResetTransitionState();
}

void UMASpaceTransitionSubsystem::HandleClientProgress(
	AMAPlayerControllerBase& PlayerController,
	const FString& DestinationInstanceIdentity,
	const bool bSucceeded)
{
	if (GetWorld()->GetNetMode() == NM_Client || !ActiveRequest.IsValid() ||
		DestinationInstanceIdentity != ActiveRequest.DestinationInstanceIdentity)
	{
		return;
	}

	if (PendingPlayers.Remove(&PlayerController) == 0) return;

	if (!bSucceeded && Phase != EPhase::Opening)
	{
		AbortTransition();
		return;
	}
	if (!bSucceeded)
	{
		UE_LOG(LogMASpaceTransition, Error,
			TEXT("A client failed to open Space transition '%s'."),
			*DestinationInstanceIdentity);
	}
	if (!PendingPlayers.IsEmpty()) return;

	switch (Phase)
	{
	case EPhase::Loading:
		TryBeginClose();
		break;
	case EPhase::Closing:
		BeginOpen();
		break;
	case EPhase::Opening:
		FinishTransition();
		break;
	default:
		break;
	}
}

bool UMASpaceTransitionSubsystem::LoadDestination(const FMASpaceTransitionRequest& Request)
{
	return LevelLoader->LoadLevel(
		Request.DestinationMap,
		Request.DestinationSlotTransform,
		Request.DestinationInstanceIdentity,
		FOnMALevelLoaded::CreateUObject(this, &UMASpaceTransitionSubsystem::HandleDestinationLoaded));
}

void UMASpaceTransitionSubsystem::HandleDestinationLoaded(AMALevelRoot* LoadedLevel)
{
	if (Phase != EPhase::Loading) return;

	DestinationLevel = LoadedLevel;
	const bool bReady =
		LoadedLevel && LoadedLevel->GetTransitionCircle() && LoadedLevel->GetDirectionalLight();
	UE_LOG(
		LogMASpaceTransition,
		Log,
		TEXT("Destination '%s' ready: %s."),
		*ActiveRequest.DestinationInstanceIdentity,
		bReady ? TEXT("true") : TEXT("false"));

	if (GetWorld()->GetNetMode() == NM_Client)
	{
		NotifyServer(bReady);
		return;
	}

	if (!bReady)
	{
		AbortTransition();
		return;
	}

	TryBeginClose();
}

void UMASpaceTransitionSubsystem::TryBeginClose()
{
	if (GetWorld()->GetNetMode() == NM_Client || Phase != EPhase::Loading ||
		!DestinationLevel.IsValid() || !PendingPlayers.IsEmpty())
	{
		return;
	}

	Phase = EPhase::Closing;
	UE_LOG(LogMASpaceTransition, Log, TEXT("Transition '%s' closing Current Space."),
		*ActiveRequest.DestinationInstanceIdentity);

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMAPlayerControllerBase* PlayerController = CastChecked<AMAPlayerControllerBase>(It->Get());
		PendingPlayers.Add(PlayerController);
	}
	for (const TWeakObjectPtr<AMAPlayerControllerBase>& PlayerController : PendingPlayers.Array())
	{
		if (Phase != EPhase::Closing) return;
		PlayerController->ClientCloseSpaceTransition();
	}

	if (Phase == EPhase::Closing && PendingPlayers.IsEmpty()) BeginOpen();
}

void UMASpaceTransitionSubsystem::BeginOpen()
{
	AMALevelRoot* Source = CurrentLevel.Get();
	AMALevelRoot* Destination = DestinationLevel.Get();
	if (!Source || !Destination)
	{
		AbortTransition();
		return;
	}

	MovePlayersToDestination(*Source, *Destination);
	UE_LOG(LogMASpaceTransition, Log, TEXT("Transition '%s' opening Destination Space."),
		*ActiveRequest.DestinationInstanceIdentity);

	Phase = EPhase::Opening;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMAPlayerControllerBase* PlayerController = CastChecked<AMAPlayerControllerBase>(It->Get());
		PendingPlayers.Add(PlayerController);
	}
	for (const TWeakObjectPtr<AMAPlayerControllerBase>& PlayerController : PendingPlayers.Array())
	{
		if (Phase != EPhase::Opening) return;
		PlayerController->ClientOpenSpaceTransition();
	}

	if (Phase == EPhase::Opening && PendingPlayers.IsEmpty()) FinishTransition();
}

void UMASpaceTransitionSubsystem::FinishTransition()
{
	UE_LOG(LogMASpaceTransition, Log, TEXT("Transition '%s' completed."),
		*ActiveRequest.DestinationInstanceIdentity);
	PromoteDestination();
	ResetTransitionState();
}

void UMASpaceTransitionSubsystem::AbortTransition()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		CastChecked<AMAPlayerControllerBase>(It->Get())->ClientAbortSpaceTransition();
	}

	if (TransitionMask) TransitionMask->Reset();
	DiscardDestination();
	ResetTransitionState();
}

void UMASpaceTransitionSubsystem::DiscardDestination()
{
	if (AMALevelRoot* Destination = DestinationLevel.Get())
	{
		LevelLoader->UnloadLevel(*Destination);
	}
	LevelLoader->CancelPendingLoad();
}

void UMASpaceTransitionSubsystem::ResetTransitionState()
{
	if (AMAPlayerCharacter* PlayerCharacter = LocalPlayerCharacter.Get())
		PlayerCharacter->GetPlayerCamera()->SetLocationLagEnabled(true);
	Phase = EPhase::Idle;
	TransitionAlpha = 1.f;
	ActiveRequest = FMASpaceTransitionRequest();
	DestinationLevel.Reset();
	LocalPlayerCharacter.Reset();
	PendingPlayers.Reset();
}

void UMASpaceTransitionSubsystem::CompleteLocalOpen(const bool bSucceeded)
{
	if (AMAPlayerCharacter* PlayerCharacter = LocalPlayerCharacter.Get())
		PlayerCharacter->GetPlayerCamera()->SetLocationLagEnabled(true);
	TransitionMask->Reset();

	const bool bPureClient = GetWorld()->GetNetMode() == NM_Client;
	if (bPureClient) PromoteDestination();

	NotifyServer(bSucceeded);
	if (bPureClient) ResetTransitionState();
}

void UMASpaceTransitionSubsystem::NotifyServer(const bool bSucceeded)
{
	if (!ActiveRequest.IsValid()) return;

	if (AMAPlayerControllerBase* PlayerController =
		Cast<AMAPlayerControllerBase>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PlayerController->ServerNotifySpaceTransitionProgress(
			ActiveRequest.DestinationInstanceIdentity,
			bSucceeded);
	}
}

void UMASpaceTransitionSubsystem::PlayTransitionSound(
	const FGameplayTag& SoundTag,
	const AMAMagicCircle& Circle) const
{
	if (UMAGameplaySoundSubsystem* SoundSubsystem =
		GetWorld()->GetSubsystem<UMAGameplaySoundSubsystem>())
	{
		SoundSubsystem->PlayAtLocation(SoundTag, Circle.GetActorLocation(), &Circle);
	}
}

void UMASpaceTransitionSubsystem::MovePlayersToDestination(
	AMALevelRoot& Source,
	AMALevelRoot& Destination) const
{
	const AMAMagicCircle* SourceCircle = Source.GetTransitionCircle();
	const AMAMagicCircle* DestinationCircle = Destination.GetTransitionCircle();
	if (!ensure(SourceCircle && DestinationCircle)) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APawn* Pawn = It->Get()->GetPawn();
		if (!Pawn) continue;

		const FTransform RelativeTransform = SourceCircle->WorldToCircleTransform(Pawn->GetActorTransform());
		const FTransform DestinationTransform = DestinationCircle->CircleToWorldTransform(RelativeTransform);
		Pawn->SetActorLocationAndRotation(
			DestinationTransform.GetLocation(),
			DestinationTransform.Rotator(),
			false,
			nullptr,
			ETeleportType::TeleportPhysics);
	}
}

void UMASpaceTransitionSubsystem::PromoteDestination()
{
	AMALevelRoot* Source = CurrentLevel.Get();
	AMALevelRoot* Destination = DestinationLevel.Get();
	if (!ensure(Source && Destination)) return;

	LevelLoader->UnloadLevel(*Source);
	CurrentLevel = Destination;
}
