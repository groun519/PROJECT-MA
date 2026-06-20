#include "Player/MAPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Widget/MAGameplayWidget.h"
#include "Widget/Battle/InBattleStageWidget.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h" 
#include "Player/MAPlayerState.h"
#include "Framework/MAGameInstance.h"
#include "Framework/MAGameMode.h"
#include "Framework/MAGameState.h"
#include "Input/MAInputStatics.h"
#include "Player/Feedback/MAFloatingTextComponent.h"
#include "Shop/MAShopNPC.h"
#include "TimerManager.h"

AMAPlayerController::AMAPlayerController()
{
	TeamID = FGenericTeamId(0);
	FloatingTextComponent = CreateDefaultSubobject<UMAFloatingTextComponent>("Floating Text Component");
}

void AMAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;
	
	if (AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr)
	{
		GS->OnMASectorStateChanged.AddUObject(this, &AMAPlayerController::HandleSectorStateChanged);
		HandleSectorStateChanged(GS->GetMASectorState());
	}	

	if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
	{
		FLoadoutSelection LoadedLoadout;
		if (GI->LoadLoadout(LoadedLoadout))
		{
			if (HasAuthority())
			{
				if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
				{
					PS->SetLoadoutSelection(LoadedLoadout);
				}
			}
			else
			{
				ServerSetLoadoutSelection(LoadedLoadout);
			}
		}
	}
}

void AMAPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		if (TeamID == FGenericTeamId::NoTeam)
		{
			TeamID = FGenericTeamId(0);
		}
		MAPlayerCharacter->ServerSideInit();
		MAPlayerCharacter->SetGenericTeamId(TeamID);
		FloatingTextComponent->BindToPawn(MAPlayerCharacter);
	}
}

void AMAPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);
	MAPlayerCharacter = Cast<AMAPlayerCharacter>(NewPawn);
	if (MAPlayerCharacter)
	{
		MAPlayerCharacter->ClientSideInit();
		FloatingTextComponent->BindToPawn(MAPlayerCharacter);
		SpawnGameplayWidget();
	}
}

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

void AMAPlayerController::ClientPlayCoinRewardFeedback_Implementation(const FMACoinRewardFeedbackParams& Params)
{
	if (!Params.RewardVFX || !Params.TargetActor) return;

	AMACoinRewardVFXActor* CoinRewardActor = GetWorld()->SpawnActor<AMACoinRewardVFXActor>(
		AMACoinRewardVFXActor::StaticClass(),
		Params.SourceLocation,
		FRotator::ZeroRotator);
	if (CoinRewardActor)
	{
		CoinRewardActor->Play(Params);
	}
}

void AMAPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController()) return;

	if (MAPlayerCharacter)
	{
		FMAInputStatics::RegisterInputMappingContextDefaults(this, MAPlayerCharacter->GetGameplayInputMappingContext());
	}

	GameplayWidget = CreateWidget<UMAGameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
		if (bHasPendingLoopReadyVisibility)
		{
			GameplayWidget->SetLoopReadyVisible(bPendingLoopReadyVisible);
			bHasPendingLoopReadyVisibility = false;
		}
	}
}

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
		EnhancedInputComp->BindAction(SkillSlotToggleInputAction, ETriggerEvent::Started, this, &AMAPlayerController::ToggleSkillSlots);
	}
}

void AMAPlayerController::NotifyInputBindingsChanged()
{
	OnInputBindingsChanged.Broadcast();
}

void AMAPlayerController::SetGameplayWidgetVisible(bool bVisible)
{
	if (GameplayWidget)
	{
		GameplayWidget->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void AMAPlayerController::RequestShopPurchase(AMAShopNPC* ShopNPC, int32 StockId)
{
	if (!ShopNPC || StockId == INDEX_NONE) return;

	if (HasAuthority())
	{
		ShopNPC->RequestPurchase(this, StockId);
		return;
	}

	ServerRequestShopPurchase(ShopNPC, StockId);
}

void AMAPlayerController::ServerRequestShopPurchase_Implementation(AMAShopNPC* ShopNPC, int32 StockId)
{
	if (!ShopNPC || StockId == INDEX_NONE) return;
	ShopNPC->RequestPurchase(this, StockId);
}

void AMAPlayerController::ToggleSkillSlots()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleSkillSlotsCollapsed();
	}
}

bool AMAPlayerController::Server_SendChatMessage_Validate(const FString& Message, EChatType ChatType)
{
	return true;
}

void AMAPlayerController::Server_SendChatMessage_Implementation(const FString& Message, EChatType ChatType)
{
	// 보낸 사람 이름
	FString SenderName = TEXT("Unknown");
	if (PlayerState)
	{
		SenderName = PlayerState->GetPlayerName();
	}

	// 조건 검사 없이 접속한 모든 사람에게
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMAPlayerController* TargetPC = Cast<AMAPlayerController>(It->Get());
		
		if (TargetPC)
		{
			// 팀 확인 없이 그냥 보냄
			TargetPC->Client_ReceiveChatMessage(SenderName, Message, ChatType);
		}
	}
}
void AMAPlayerController::Client_ReceiveChatMessage_Implementation(const FString& SenderName, const FString& Message, EChatType ChatType)
{
	// UI에게 알림 방송 (이전에 작성한 코드)
	OnChatMessageReceived.Broadcast(SenderName, Message, ChatType);
}

void AMAPlayerController::ServerSetLoadoutSelection_Implementation(const FLoadoutSelection& Loadout)
{
	if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
	{
		PS->SetLoadoutSelection(Loadout);
	}
}

void AMAPlayerController::ServerSetLoopReady_Implementation(bool bReady)
{
	if (AMAGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AMAGameMode>() : nullptr)
	{
		GM->SetPlayerLoopReady(GetPlayerState<APlayerState>(), bReady);
	}
}

void AMAPlayerController::HandleSectorStateChanged(EMASectorState NewState)
{
	const bool bShowLoopReady = (NewState == EMASectorState::Loop);
	if (GameplayWidget)
	{
		GameplayWidget->SetLoopReadyVisible(bShowLoopReady);
	}
	else
	{
		bHasPendingLoopReadyVisibility = true;
		bPendingLoopReadyVisible = bShowLoopReady;
	}

	if (NewState == EMASectorState::InBattle)
	{
		ShowInBattleStageWidget();
	}
}

void AMAPlayerController::ShowInBattleStageWidget()
{
	if (!IsLocalController()) return;

	if (!InBattleStageWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("InBattleStageWidgetClass is not set."));
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(InBattleStageWidgetTimer);
	}

	if (InBattleStageWidget)
	{
		InBattleStageWidget->RemoveFromParent();
		InBattleStageWidget = nullptr;
	}

	InBattleStageWidget = CreateWidget<UInBattleStageWidget>(this, InBattleStageWidgetClass);
	if (!InBattleStageWidget) return;

	InBattleStageWidget->AddToViewport();

	if (AMAGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr)
	{
		const FStageCycle& StageCycle = GS->GetStageCycle();
		const FString StageText = FString::Printf(TEXT("%d-%d"), StageCycle.Round, StageCycle.Stage);
		InBattleStageWidget->SetStageText(FText::FromString(StageText));
	}

	InBattleStageWidget->PlayShowAnimation();

	const float Duration = InBattleStageWidget->GetShowAnimationDuration();
	const float RemoveDelay = (Duration > 0.0f) ? Duration : 2.0f;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			InBattleStageWidgetTimer,
			this,
			&AMAPlayerController::RemoveInBattleStageWidget,
			RemoveDelay,
			false
		);
	}
}

void AMAPlayerController::RemoveInBattleStageWidget()
{
	if (InBattleStageWidget)
	{
		InBattleStageWidget->RemoveFromParent();
		InBattleStageWidget = nullptr;
	}
}
