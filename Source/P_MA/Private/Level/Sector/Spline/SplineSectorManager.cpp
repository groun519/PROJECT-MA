// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineSectorManager.h"
#include "DrawDebugHelpers.h"
#include "Framework/MAGameMode.h"
#include "Framework/MAGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Level/Platform/Core.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "TimerManager.h"

ASplineSectorManager::ASplineSectorManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASplineSectorManager::BeginPlay()
{
	Super::BeginPlay();

	PlatformRoot = Cast<APlatformRoot>(
	UGameplayStatics::GetActorOfClass(GetWorld(), APlatformRoot::StaticClass())
	);

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
		
		/** PlatformRoot **/
		APlatformRoot* PR = Cast<APlatformRoot>(UGameplayStatics::GetActorOfClass(GetWorld(), APlatformRoot::StaticClass()));
		if (PR)
		{
			CachedPlatformRoot = PR;
			CachedPlatformRoot->OnPlatformReachedEnd.AddUObject(this, &ASplineSectorManager::OnHandlePlatformReachedEnd);
			if (CachedMAGameMode)
			{
				bool bWaitMoveIn =
					CachedMASectorState == EMASectorState::Wait || CachedMASectorState == EMASectorState::EndBattle;
				CachedPlatformRoot->SetWaitMoveIn(bWaitMoveIn);
				CachedPlatformRoot->SetHeight(bIsMoving);
			}
		}

		if (PlayerRangeClamp.bUse && PlayerRangeClamp.Interval > 0.f)
		{
			GetWorldTimerManager().SetTimer(
				PlayerRangeClampTimerHandle,
				this,
				&ASplineSectorManager::UpdatePlayerRangeClamp,
				PlayerRangeClamp.Interval,
				true);
		}

		UpdatePlayerRangeClampVisual();
	}
}

void ASplineSectorManager::OnHandleSectorStateChanged(EMASectorState NewState)
{
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
		
		if (CachedPlatformRoot && bIsMoving && !CurSectors.IsEmpty() && CurSectors[0])
		{
			USplineComponent* CurSpline = CurSectors[0]->RoadSpline;
			if (IsValid(CurSpline)) CachedPlatformRoot->SetCurSpline(CurSpline);
		}
	}

	// 스테이트 캐시
	CachedMASectorState = NewState;
	UpdatePlayerRangeClampVisual();
	
	if (CachedPlatformRoot)
	{
		bool bWaitMoveIn =
			NewState == EMASectorState::Wait || NewState == EMASectorState::EndBattle;
		CachedPlatformRoot->SetWaitMoveIn(bWaitMoveIn);
		CachedPlatformRoot->SetHeight(bIsMoving);

		if (ACore* Core = CachedPlatformRoot->GetCore())
		{
			const bool bIsBattle = (NewState == EMASectorState::Battle);
			Core->ApplyBattleColor(bIsBattle);
		}
	}

	if (DebugSetting.bUseStateDebug)
	UE_LOG(LogTemp, Warning, TEXT("SplineManager: 상태 변화 감지 -> %d"), (int32)NewState);
}

void ASplineSectorManager::OnHandlePlatformReachedEnd()
{
	if (CurSectors.IsEmpty() || !CachedPlatformRoot) return;

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
	if (!CachedPlatformRoot) return;
	CachedPlatformRoot->SetReadyText(ReadyCount, TotalCount);
}

int32 ASplineSectorManager::GetNextSectorIndex(int32 InSectorIndex)
{
	int32 LastSectorIndex = CurSectors.Num() - 1;
	if (InSectorIndex == LastSectorIndex)
	{
		return 0;
	}
	else
	{
		return InSectorIndex + 1;
	}
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

void ASplineSectorManager::SetSectorsByState(EMASectorState InState)
{
	FSplineSectorData SSData = SplineSectorsByState[InState];
	
	bIsMoving = SSData.bIsMoving;

	if (PlatformRoot)
	{
		if (SSData.MoveInState == EMoveInState::Nothing)
		{
			PlatformRoot->SetWaitMoveIn(false);
		}
		else if (SSData.MoveInState == EMoveInState::CanMoveIn)
		{
			PlatformRoot->SetWaitMoveIn(true);
		}
		else if (SSData.MoveInState == EMoveInState::CanMoveOut)
		{
			PlatformRoot->SetWaitMoveIn(false);
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
	if (!CachedPlatformRoot) return;

	// If Stop Sector, assign nullptr to CurSpline
	if (!bIsMoving || CurSectors.IsEmpty() || !CurSectors[CurSectorIndex])
	{
		CachedPlatformRoot->SetCurSpline(nullptr);
		return;
	}
	USplineComponent* CurSpline = CurSectors[CurSectorIndex]->RoadSpline;
	if (!IsValid(CurSpline))
	{
		CachedPlatformRoot->SetCurSpline(nullptr);
		return;
	}

	// Change Seed
	const int32 NextIndex = GetNextSectorIndex(CurSectorIndex);
	const int32 PrevIndex = (CurSectorIndex == 0) ? (CurSectors.Num() - 1) : (CurSectorIndex - 1);
	const bool bNextIsJustPassedSector = (NextIndex == PrevIndex);
	if (!bNextIsJustPassedSector && CurSectors.IsValidIndex(NextIndex) && CurSectors[NextIndex])
	{
		CurSectors[NextIndex]->SetRandomSeed();
	}

	// Set Spline
	CachedPlatformRoot->SetCurSpline(CurSpline);
}

void ASplineSectorManager::ApplyRegenTargetsOnEnter(const FSplineSectorData& InData)
{
	if (!HasAuthority()) return;

	for (ASplineSector* RegenTarget : InData.RegenTargetsOnEnter)
	{
		if (!RegenTarget) continue;
		RegenTarget->SetRandomSeed();
	}
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
	if (!CachedPlatformRoot) return;

	const bool bVisible = CanApplyPlayerRangeClamp();
	const float Size = FMath::Max(0.f, PlayerRangeClamp.Radius);
	CachedPlatformRoot->SetRangeClampVisual(bVisible, Size);
}

void ASplineSectorManager::UpdatePlayerRangeClamp()
{
	if (!HasAuthority()) return;
	if (!CanApplyPlayerRangeClamp()) return;
	if (!CachedPlatformRoot) return;

	AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr;
	if (!GS) return;

	TArray<AMAPlayerCharacter*> Players;
	GS->GetPlayerCharacters(Players, true);
	if (Players.IsEmpty()) return;

	const FVector Center = CachedPlatformRoot->GetActorLocation();
	const float ClampRadiusWithDeadZone = PlayerRangeClamp.Radius + PlayerRangeClamp.DeadZone;
	const float ClampRadiusWithDeadZoneSq = FMath::Square(ClampRadiusWithDeadZone);

	for (AMAPlayerCharacter* Player : Players)
	{
		if (!Player) continue;

		const FVector PlayerLoc = Player->GetActorLocation();
		FVector Delta = PlayerLoc - Center;
		Delta.Z = 0.f;

		if (Delta.SizeSquared() <= ClampRadiusWithDeadZoneSq) continue;

		const FVector ClampedLoc2D = Center + Delta.GetSafeNormal() * PlayerRangeClamp.Radius;
		const FVector ClampedLoc = FVector(ClampedLoc2D.X, ClampedLoc2D.Y, PlayerLoc.Z);

		Player->SetActorLocation(ClampedLoc, false, nullptr, ETeleportType::TeleportPhysics);
		if (UCharacterMovementComponent* MoveComp = Player->GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
		}
	}
}

bool ASplineSectorManager::CanApplyPlayerRangeClamp() const
{
	if (!PlayerRangeClamp.bUse) return false;
	if (PlayerRangeClamp.Radius <= 0.f) return false;

	if (PlayerRangeClamp.States.IsEmpty())
	{
		return true;
	}

	return PlayerRangeClamp.States.Contains(CachedMASectorState);
}
