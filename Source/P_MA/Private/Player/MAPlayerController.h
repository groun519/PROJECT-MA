// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "Framework/MAGameStateTypes.h"
#include "MAPlayerController.generated.h"

UENUM(BlueprintType)
enum class EChatType : uint8
{
	Normal		UMETA(DisplayName = "Normal"), // 일반 채팅 
	System		UMETA(DisplayName = "System")  // 시스템 공지
};
// UI 알림용 델리게이트 정의
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatMessageReceived, const FString&, SenderName, const FString&, Message, EChatType, ChatType);

/**
 * 
 */
UCLASS()
class AMAPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AMAPlayerController();

	// 서버에서만 호출됨
	void OnPossess(APawn* NewPawn) override;
	// 클라이언트에서만 호출됨, 리슨서버도.
	void AcknowledgePossession(APawn* NewPawn) override;

	virtual void BeginPlay() override;
	
	// TeamID에 Team Agent 할당
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	// FGenericTeamId 형식으로 TeamID 탐색
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerNotifyLoaded();

	/** Loadout **/
	UFUNCTION(Server, Reliable)
	void ServerSetLoadoutSelection(const FLoadoutSelection& Loadout);

	/** LoopReady **/
	UFUNCTION(Server, Reliable)
	void ServerSetLoopReady(bool bReady);

	/** ChatMessage **/
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceived OnChatMessageReceived;
	
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Chat")
	void Server_SendChatMessage(const FString& Message, EChatType ChatType);

	UFUNCTION(Client, Reliable, Category = "Chat")
	void Client_ReceiveChatMessage(const FString& SenderName, const FString& Message, EChatType ChatType);

private:
	void SpawnGameplayWidget();
	void HandleSectorStateChanged(EMASectorState NewState);
	void ShowInBattleStageWidget();
	void RemoveInBattleStageWidget();

	UPROPERTY()
	class AMAPlayerCharacter* MAPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UMAGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	class UMAGameplayWidget* GameplayWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UInBattleStageWidget> InBattleStageWidgetClass;

	UPROPERTY()
	class UInBattleStageWidget* InBattleStageWidget;

	FTimerHandle InBattleStageWidgetTimer;

	bool bHasPendingLoopReadyVisibility = false;
	bool bPendingLoopReadyVisible = false;
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* UIInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ShopToggleInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* SkillBookToggleInputAction;

	UFUNCTION()
	void ToggleShop();
	
	UFUNCTION()
	void ToggleSkillBook();
};
