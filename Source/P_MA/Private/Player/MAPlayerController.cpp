// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MAPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Player/MAPlayerCharacter.h"
#include "Widget/MAGameplayWidget.h"
#include "Net/UnrealNetwork.h"

void AMAPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		MAPlayerCharacter->ServerSideInit();
		MAPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void AMAPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		MAPlayerCharacter->ClientSideInit();
		SpawnGameplayWidget();
	}
	/** 아래는 별로 코드입니다 **/
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
	/** 위에까지는 별로 코드입니다 **/
}

/** 아래는 별로 코드입니다 **/
void AMAPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckMouseCursorShape(); 
}
/** 위에까지는 별로 코드입니다 **/

void AMAPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId AMAPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void AMAPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAPlayerController, TeamID);
}

void AMAPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController()) return;

	GameplayWidget = CreateWidget<UMAGameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
	}
}

/** 여기에는 강의에는 없는 별도 코드입니다 **/
void AMAPlayerController::CheckMouseCursorShape()
{
	FHitResult mouseHitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, mouseHitResult);

	if (mouseHitResult.bBlockingHit)
	{
		AActor* hitActor = mouseHitResult.GetActor();

		if (hitActor && hitActor->IsA(AMACharacter::StaticClass()))
		{
			if (!bOnMouseCursorRecord)
			{
				bOnMouseCursorRecord = true;
				CurrentMouseCursor = EMouseCursor::Crosshairs;
			}
			return;
		}

		// 다른 액터지만 몬스터가 아닐 때 → 기본 커서로
		if (bOnMouseCursorRecord)
		{
			bOnMouseCursorRecord = false;
			CurrentMouseCursor = EMouseCursor::Default;
		}
	}
	else
	{
		// 아무 것도 안 맞았을 때도 기본 커서로 돌려주기
		if (bOnMouseCursorRecord)
		{
			bOnMouseCursorRecord = false;
			CurrentMouseCursor = EMouseCursor::Default;
		}
	}
}
/** 여기 위에 까지에는 강의에는 없는 별도 코드입니다 **/

