#include "SplineSectorManager.h"

#include "DrawDebugHelpers.h"
#include "Framework/MAGameMode.h"
#include "Framework/MAGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Level/Environment/EnvironmentManager.h"
#include "PCGGraph.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerControllerBase.h"
#include "Player/Camera/MAPlayerCameraDirectorComponent.h"
#include "TimerManager.h"

ASplineSectorManager::ASplineSectorManager()
{
	PrimaryActorTick.bCanEverTick = false;

	FPlayerRangeClampSettings DefaultClampSetting;
	PlayerRangeClampByState.Add(EMASectorState::Wait, DefaultClampSetting);
	PlayerRangeClampByState.Add(EMASectorState::EndBattle, DefaultClampSetting);
	PlayerRangeClampByState.Add(EMASectorState::Loop, DefaultClampSetting);
}

void ASplineSectorManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		/** GameMode **/
		AMAGameMode* MAGM = Cast<AMAGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (MAGM)
		{
			CachedMAGameMode = MAGM;
			CachedMAGameMode->OnMASectorStateChanged.AddUObject(this, &ASplineSectorManager::OnHandleSectorStateChanged);
			OnHandleSectorStateChanged(CachedMAGameMode->GetMASectorState());
			CachedMAGameMode->OnReadyCountChanged.AddUObject(this, &ASplineSectorManager::OnHandleReadyCountChanged);
		}
		
		/** RideRoot **/
		ARideRoot* RR = Cast<ARideRoot>(UGameplayStatics::GetActorOfClass(GetWorld(), ARideRoot::StaticClass()));
		if (RR)
		{
			CachedRideRoot = RR;
			CachedRideRoot->OnPlatformReachedEnd.AddUObject(this, &ASplineSectorManager::OnHandlePlatformReachedEnd);
			if (CachedMAGameMode)
			{
				bool bWaitMoveIn =
					CachedMASectorState == EMASectorState::Wait || CachedMASectorState == EMASectorState::EndBattle;
				CachedRideRoot->SetWaitMoveIn(bWaitMoveIn);
			}
		}

		if (PlayerRangeClampInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(
				PlayerRangeClampTimerHandle,
				this,
				&ASplineSectorManager::UpdatePlayerRangeClamp,
				PlayerRangeClampInterval,
				true);
		}

		UpdatePlayerRangeClampVisual();
	}

	BindEnvironmentManager();
}

void ASplineSectorManager::OnHandleSectorStateChanged(EMASectorState NewState)
{
	CancelReadyCountdown();
	GetWorldTimerManager().ClearTimer(LoopReadyCompletionTimerHandle);
	LogStateChange(NewState);
	bool bWasMoving = bIsMoving;
	
	// Set IsMoving
	FSplineSectorData SSData = SplineSectorsByState[NewState];
	bIsMoving = SSData.bIsMoving;
	bIsAutoPass = SSData.bIsAutoPass;
	ApplyRegenTargetsOnEnter(SSData);

	// 만약 이전 상태가 Start였다면,
	// 스플라인의 끝에 도달하지 못하는 상태기에 한번 ApplyCurSplineAndSeed를 실행하여 게임 루프를 시작시킴.
	if (bWasMoving == false && bIsMoving == true)
	{
		CurSectorIndex = 0;
		SetSectorsByState(NewState);
		
		if (CachedRideRoot && bIsMoving && !CurSectors.IsEmpty() && CurSectors[0])
		{
			USplineComponent* CurSpline = CurSectors[0]->RoadSpline;
			if (IsValid(CurSpline)) CachedRideRoot->SetCurSpline(CurSpline);
		}
	}

	// 스테이트 캐시
	CachedMASectorState = NewState;
	UpdatePlayerRangeClampVisual();
	if (HasAuthority())
	{
		// Apply clamp immediately on state transition instead of waiting for next timer tick.
		UpdatePlayerRangeClamp();
	}
	
	if (CachedRideRoot)
	{
		bool bWaitMoveIn =
			NewState == EMASectorState::Wait || NewState == EMASectorState::EndBattle;
		CachedRideRoot->SetWaitMoveIn(bWaitMoveIn);
	}

	if (DebugSetting.bUseStateDebug)
	UE_LOG(LogTemp, Warning, TEXT("SplineManager: 상태 변화 감지 -> %d"), (int32)NewState);
}

void ASplineSectorManager::OnHandlePlatformReachedEnd()
{
	if (CurSectors.IsEmpty() || !CachedRideRoot) return;

	// 마지막 스테이트인가 ?
	bool bIsLastSector = CurSectorIndex >= CurSectors.Num() - 1;
	if (bIsLastSector)
	{
		// 만약 자동으로 넘겨야 하는 스테이트라면 넘김.
		IsAutoPassState(CachedMASectorState);

		// 섹터 세팅
		SetSectorsByState(CachedMASectorState);
		CurSectorIndex = 0;
		
		// 스테이트 넘어갈 때 로그 찍기
		LogStateChange(CachedMASectorState);
	}
	else
	{
		CurSectorIndex++;
	}
	
	// 타고 갈 스플라인을 적용, 다음 섹터 시드 변경.
	ApplyCurSplineAndSeed();

	if (DebugSetting.bUseSplineEndTimeDebug)
	UE_LOG(LogTemp, Warning, TEXT("SplineManager: 플랫폼 섹터 끝 도달!"));
}

void ASplineSectorManager::OnHandleReadyCountChanged(int32 ReadyCount, int32 TotalCount)
{
	if (!CachedRideRoot) return;

	if (CanUseReadyCountdown() && ReadyCount > 0 && ReadyCount == TotalCount)
	{
		StartReadyCountdown();
		return;
	}

	CancelReadyCountdown();
	CachedRideRoot->SetReadyText(ReadyCount, TotalCount);
}

void ASplineSectorManager::CompleteLoopReady()
{
	if (!HasAuthority() || !CachedRideRoot || !CachedMAGameMode || !GetWorld()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMAPlayerControllerBase* PC = Cast<AMAPlayerControllerBase>(It->Get());
		if (!PC) continue;

		if (UMAPlayerCameraDirectorComponent* CameraDirector = PC->GetCameraDirector())
		{
			CameraDirector->RequestFade(LoopReadyFadeSettings);
		}
	}

	GetWorldTimerManager().SetTimer(
		LoopReadyCompletionTimerHandle,
		[this]()
		{
			if (!HasAuthority() || !CachedRideRoot || !CachedMAGameMode) return;

			const EMASectorState NewState = EMASectorState::InBattle;
			CachedMAGameMode->RequestStateChange(NewState);
			CurSectorIndex = 0;
			SetSectorsByState(NewState);
			ApplyCurSplineAndSeed();
			CachedRideRoot->ApplyCurrentSplineTransform();
		},
		LoopReadyFadeSettings.FadeOutSeconds,
		false);
}

int32 ASplineSectorManager::GetNextSectorIndex(int32 InSectorIndex)
{
	int32 LastSectorIndex = CurSectors.Num() - 1;
	if (InSectorIndex == LastSectorIndex) return 0;
	
	return InSectorIndex + 1;
}

EMASectorState ASplineSectorManager::GetMASectorState() const
{
	if (CachedMAGameMode)
	{
		return CachedMAGameMode->GetMASectorState();
	}
	return EMASectorState::Wait;
}

ASplineSectorManager* ASplineSectorManager::FindSplineSectorManager(UWorld* World)
{
	AActor* Found = UGameplayStatics::GetActorOfClass(World, ASplineSectorManager::StaticClass());
	ASplineSectorManager* SSM = Cast<ASplineSectorManager>(Found);
	return SSM;
}

bool ASplineSectorManager::CanUseReadyCountdown() const
{
	return CachedMAGameMode &&
		(CachedMASectorState == EMASectorState::Wait || CachedMASectorState == EMASectorState::EndBattle);
}

void ASplineSectorManager::StartReadyCountdown()
{
	if (!CachedRideRoot || !CachedMAGameMode) return;
	if (GetWorldTimerManager().IsTimerActive(ReadyCountdownTimerHandle)) return;

	if (ReadyStartDelay <= 0.f)
	{
		CachedMAGameMode->RequestNextState(CachedMAGameMode->GetMASectorState());
		return;
	}

	ReadyCountdownRemainingSeconds = FMath::Max(1, FMath::CeilToInt(ReadyStartDelay));
	TickReadyCountdown();
	GetWorldTimerManager().SetTimer(
		ReadyCountdownTimerHandle,
		this,
		&ASplineSectorManager::TickReadyCountdown,
		1.f,
		true);
}

void ASplineSectorManager::CancelReadyCountdown()
{
	GetWorldTimerManager().ClearTimer(ReadyCountdownTimerHandle);
	ReadyCountdownRemainingSeconds = 0;
}

void ASplineSectorManager::TickReadyCountdown()
{
	if (!CachedRideRoot || !CachedMAGameMode) return;

	if (ReadyCountdownRemainingSeconds <= 0)
	{
		GetWorldTimerManager().ClearTimer(ReadyCountdownTimerHandle);
		CachedMAGameMode->RequestNextState(CachedMAGameMode->GetMASectorState());
		return;
	}

	CachedRideRoot->SetReadyCountdownText(ReadyCountdownRemainingSeconds);
	--ReadyCountdownRemainingSeconds;
}

void ASplineSectorManager::SetSectorsByState(EMASectorState InState)
{
	FSplineSectorData SSData = SplineSectorsByState[InState];
	
	bIsMoving = SSData.bIsMoving;

	if (CachedRideRoot)
	{
		if (SSData.MoveInState == EMoveInState::Nothing)
		{
			CachedRideRoot->SetWaitMoveIn(false);
		}
		else if (SSData.MoveInState == EMoveInState::CanMoveIn)
		{
			CachedRideRoot->SetWaitMoveIn(true);
		}
		else if (SSData.MoveInState == EMoveInState::CanMoveOut)
		{
			CachedRideRoot->SetWaitMoveIn(false);
			CachedRideRoot->ReleaseAttachedPlayers();
			if (CachedMAGameMode)
			{
				CachedMAGameMode->ResetAllPlayersReady();
			}
		}
	}
	
	if (bIsMoving)
		CurSectors = SSData.Sectors;
	else
		CurSectors.Empty();
}

bool ASplineSectorManager::IsAutoPassState(EMASectorState InState)
{
	int32 StateNum = static_cast<int32>(InState);
	if (StateNum == 6) return false;
	
	if (bIsAutoPass)
	{
		if (CachedMAGameMode) CachedMAGameMode->RequestNextState(InState);
		return true;
	}
	return false;
}

void ASplineSectorManager::ApplyCurSplineAndSeed()
{
	if (!CachedRideRoot) return;

	// If Stop Sector, assign nullptr to CurSpline
	if (!bIsMoving || CurSectors.IsEmpty() || !CurSectors[CurSectorIndex])
	{
		CachedRideRoot->SetCurSpline(nullptr);
		return;
	}
	USplineComponent* CurSpline = CurSectors[CurSectorIndex]->RoadSpline;
	if (!IsValid(CurSpline))
	{
		CachedRideRoot->SetCurSpline(nullptr);
		return;
	}

	// Change Seed
	const int32 NextIndex = GetNextSectorIndex(CurSectorIndex);
	const int32 PrevIndex = (CurSectorIndex == 0) ? (CurSectors.Num() - 1) : (CurSectorIndex - 1);
	const bool bNextIsJustPassedSector = (NextIndex == PrevIndex);
	if (!bNextIsJustPassedSector && CurSectors.IsValidIndex(NextIndex) && CurSectors[NextIndex])
	{
		ApplyCachedEnvironmentToSector(CurSectors[NextIndex]);
		CurSectors[NextIndex]->SetRandomSeed();
	}

	// Set Spline
	CachedRideRoot->SetCurSpline(CurSpline);
}

void ASplineSectorManager::ApplyRegenTargetsOnEnter(const FSplineSectorData& InData)
{
	if (!HasAuthority()) return;

	for (ASplineSector* RegenTarget : InData.RegenTargetsOnEnter)
	{
		if (!RegenTarget) continue;
		ApplyCachedEnvironmentToSector(RegenTarget);
		RegenTarget->SetRandomSeed();
	}
}

void ASplineSectorManager::ApplyCachedEnvironmentToSector(ASplineSector* InSector) const
{
	if (!InSector || !InSector->PCGComponent) return;
	UPCGGraph* TargetPCGGraph = CachedEnvPCGGraph.Get();
	if (!TargetPCGGraph) return;
	if (InSector->PCGComponent->GetGraph() == TargetPCGGraph) return;

	InSector->PCGComponent->SetGraph(TargetPCGGraph);
}

void ASplineSectorManager::LogStateChange(EMASectorState InState) const
{
	if (!DebugSetting.bUseStateDebug) return;

	const UEnum* EnumPtr = StaticEnum<EMASectorState>();
	const FString PrevName = EnumPtr->GetNameStringByValue((int64)CachedMASectorState);
	const FString CurrName = EnumPtr->GetNameStringByValue((int64)InState);
	UE_LOG(LogTemp, Display, TEXT("PrevState: %s"), *PrevName);
	UE_LOG(LogTemp, Display, TEXT("CurrState: %s"), *CurrName);
	UE_LOG(LogTemp, Display, TEXT("- - - - -"));
}

void ASplineSectorManager::UpdatePlayerRangeClampVisual()
{
	if (!CachedRideRoot) return;

	const bool bVisible = CanApplyPlayerRangeClamp();
	const FPlayerRangeClampSettings* ClampSettings = GetPlayerRangeClampSettingsForState(CachedMASectorState);
	const float Size = ClampSettings ? FMath::Max(0.f, ClampSettings->Radius) : 0.f;
	CachedRideRoot->SetRangeClampVisual(bVisible, Size);
}

void ASplineSectorManager::UpdatePlayerRangeClamp()
{
	if (!HasAuthority()) return;
	if (!CanApplyPlayerRangeClamp()) return;
	if (!CachedRideRoot) return;

	AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr;
	if (!GS) return;

	TArray<AMAPlayerCharacter*> Players;
	GS->GetPlayerCharacters(Players, true);
	if (Players.IsEmpty()) return;

	const FPlayerRangeClampSettings* ClampSettings = GetPlayerRangeClampSettingsForState(CachedMASectorState);
	if (!ClampSettings) return;

	const FVector Center = CachedRideRoot->GetActorLocation();
	const float ClampRadiusWithDeadZone = ClampSettings->Radius + ClampSettings->DeadZone;
	const float ClampRadiusWithDeadZoneSq = FMath::Square(ClampRadiusWithDeadZone);

	for (AMAPlayerCharacter* Player : Players)
	{
		if (!Player) continue;

		const FVector PlayerLoc = Player->GetActorLocation();
		FVector Delta = PlayerLoc - Center;
		Delta.Z = 0.f;

		if (Delta.SizeSquared() <= ClampRadiusWithDeadZoneSq) continue;

		const FVector ClampedLoc2D = Center + Delta.GetSafeNormal() * ClampSettings->Radius;
		const FVector ClampedLoc = FVector(ClampedLoc2D.X, ClampedLoc2D.Y, PlayerLoc.Z);

		Player->TeleportTo(ClampedLoc, Player->GetActorRotation(), false, true);
		if (UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
		}
		Player->ForceNetUpdate();
	}
}

bool ASplineSectorManager::CanApplyPlayerRangeClamp() const
{
	const FPlayerRangeClampSettings* ClampSettings = GetPlayerRangeClampSettingsForState(CachedMASectorState);
	if (!ClampSettings) return false;
	if (!ClampSettings->bUse) return false;
	return ClampSettings->Radius > 0.f;
}

const FPlayerRangeClampSettings* ASplineSectorManager::GetPlayerRangeClampSettingsForState(EMASectorState InState) const
{
	return PlayerRangeClampByState.Find(InState);
}

bool ASplineSectorManager::BindEnvironmentManager()
{
	AEnvironmentManager* EnvironmentManager = AEnvironmentManager::FindEnvironmentManager(GetWorld());
	if (!EnvironmentManager) return false;

	EnvironmentManager->OnEnvironmentPCGChanged.AddUObject(this, &ASplineSectorManager::OnHandleEnvironmentPCGChanged);
	EnvironmentManager->BroadcastCurrentEnvironment();
	return true;
}

void ASplineSectorManager::OnHandleEnvironmentPCGChanged(UPCGGraph* NewPCGGraph)
{
	if (CachedEnvPCGGraph == NewPCGGraph) return;
	CachedEnvPCGGraph = NewPCGGraph;

	for (const TPair<EMASectorState, FSplineSectorData>& Pair : SplineSectorsByState)
	{
		const FSplineSectorData& Data = Pair.Value;
		for (ASplineSector* Sector : Data.Sectors)
		{
			ApplyCachedEnvironmentToSector(Sector);
		}
		for (ASplineSector* RegenTarget : Data.RegenTargetsOnEnter)
		{
			ApplyCachedEnvironmentToSector(RegenTarget);
		}
	}

	// One-shot startup pass: run explicit targets once after initial env is resolved.
	if (!bAppliedEnvironmentReadyRegen && CachedEnvPCGGraph)
	{
		for (ASplineSector* Sector : RegenTargetsOnEnvironmentReady)
		{
			if (!Sector) continue;
			ApplyCachedEnvironmentToSector(Sector);
			if (HasAuthority())
			{
				Sector->SetRandomSeed();
			}
			else
			{
				Sector->RegenerateWithCurrentSeed();
			}
		}
		bAppliedEnvironmentReadyRegen = true;
	}

	// State-entry-only regen targets are not touched by path progression,
	// so refresh them immediately when environment PCG changes.
	if (!HasAuthority()) return;

	const FSplineSectorData* CurrentStateData = SplineSectorsByState.Find(CachedMASectorState);
	if (!CurrentStateData) return;
	ApplyRegenTargetsOnEnter(*CurrentStateData);
}
