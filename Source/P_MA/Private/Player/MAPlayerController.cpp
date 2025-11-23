// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MAPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Player/MAPlayerCharacter.h"
#include "Widget/MAGameplayWidget.h"
#include "Widget/SkillBookWidget.h" // 디버깅을 위해
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
		GameplayWidget->ConfigureAbilities(MAPlayerCharacter->GetAbilities());
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

void AMAPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputSubsystem)
	{
		InputSubsystem->RemoveMappingContext(UIInputMapping);
		InputSubsystem->AddMappingContext(UIInputMapping, 1);
	}

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(ShopToggleInputAction, ETriggerEvent::Triggered, this, &AMAPlayerController::ToggleShop);
		EnhancedInputComp->BindAction(SkillBookToggleInputAction, ETriggerEvent::Started, this, &AMAPlayerController::ToggleSkillBook);
	}
}

void AMAPlayerController::ToggleShop()
{	
	if(GameplayWidget)
	{
		GameplayWidget->ToggleShop();
	}
}

void AMAPlayerController::ToggleSkillBook()
{
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] ToggleSkillBook Function Called! (Key Input Received)"));

	if (!GameplayWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[DEBUG] GameplayWidget is NULL! Check SpawnGameplayWidget() or Blueprint Class settings."));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Found GameplayWidget. Trying to toggle SkillBook..."));
	
	GameplayWidget->ToggleSkillBook();
	
	if (USkillBookWidget* SkillBook = GameplayWidget->GetSkillBookWidget())
	{
		bool bIsVisible = SkillBook->GetVisibility() == ESlateVisibility::Visible;
		FString StateStr = bIsVisible ? TEXT("Visible") : TEXT("Hidden");
		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] SkillBookWidget Found! Current State: %s"), *StateStr);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[DEBUG] SkillBookWidget is NULL in GameplayWidget! Check Widget Blueprint Name (must be 'SkillBookWidget')."));
	}
}