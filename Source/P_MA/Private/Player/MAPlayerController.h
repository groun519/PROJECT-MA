#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Player/MAPlayerControllerBase.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "Framework/MAGameStateTypes.h"
#include "Player/Feedback/MACoinRewardVFXActor.h"
#include "MAPlayerController.generated.h"

class AMAShopNPC;
class AMAEnchanterNPC;
class UMAFloatingTextComponent;
class UMAPlayerGameOverComponent;
class UMAPlayerSpectateComponent;
class UMASkillModuleInstance;

UENUM(BlueprintType)
enum class EChatType : uint8
{
	Normal		UMETA(DisplayName = "Normal"), // 일반 채팅 
	System		UMETA(DisplayName = "System")  // 시스템 공지
};
// 채팅UI 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChatMessageReceived, const FString&, SenderName, const FString&, Message, EChatType, ChatType);
// 바인딩 변경 델리게이트
DECLARE_MULTICAST_DELEGATE(FMAInputBindingsChangedSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FMAEnchantCompletedSignature, bool);

UCLASS()
class AMAPlayerController : public AMAPlayerControllerBase, public IGenericTeamAgentInterface
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

	FMAInputBindingsChangedSignature OnInputBindingsChanged;
	void NotifyInputBindingsChanged();
	void SetGameplayWidgetVisible(bool bVisible);
	UMAFloatingTextComponent* GetFloatingTextComponent() const { return FloatingTextComponent; }

	/** Shop **/
	void RequestShopPurchase(AMAShopNPC* ShopNPC, int32 StockId);

	UFUNCTION(Server, Reliable)
	void ServerRequestShopPurchase(AMAShopNPC* ShopNPC, int32 StockId);

	/** Enchantment **/
	FMAEnchantCompletedSignature OnEnchantCompleted;

	void RequestEnchantModule(
		AMAEnchanterNPC* EnchanterNPC,
		UMASkillModuleInstance* TargetModule,
		int32 RuneEntryId,
		int32 EnchantmentSlotIndex);

	UFUNCTION(Client, Reliable)
	void ClientCompleteEnchant(bool bSucceeded);

	UFUNCTION(Server, Reliable)
	void ServerRequestEnchantModule(
		AMAEnchanterNPC* EnchanterNPC,
		UMASkillModuleInstance* TargetModule,
		int32 RuneEntryId,
		int32 EnchantmentSlotIndex);

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

	UFUNCTION(Client, Unreliable)
	void ClientPlayCoinRewardFeedback(const FMACoinRewardFeedbackParams& Params);
	
private:
	void SpawnGameplayWidget();
	void HandleSectorStateChanged(EMASectorState NewState);
	void ShowInBattleStageWidget();
	void RemoveInBattleStageWidget();

	UPROPERTY()
	class AMAPlayerCharacter* MAPlayerCharacter;

	UPROPERTY(VisibleDefaultsOnly, Category="Feedback")
	TObjectPtr<UMAFloatingTextComponent> FloatingTextComponent;

	UPROPERTY(VisibleDefaultsOnly, Category="Spectate")
	TObjectPtr<UMAPlayerSpectateComponent> SpectateComponent;

	UPROPERTY(VisibleDefaultsOnly, Category="Game Over")
	TObjectPtr<UMAPlayerGameOverComponent> GameOverComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UMAGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	UMAGameplayWidget* GameplayWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UInBattleStageWidget> InBattleStageWidgetClass;

	UPROPERTY()
	UInBattleStageWidget* InBattleStageWidget;

	FTimerHandle InBattleStageWidgetTimer;

	bool bHasPendingLoopReadyVisibility = false;
	bool bPendingLoopReadyVisible = false;
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* UIInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* SkillSlotToggleInputAction;

	void ToggleSkillSlots();
};
