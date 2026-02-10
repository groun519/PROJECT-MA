// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "Framework/MAGameStateTypes.h"
#include "MAPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMAPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// 서버에서만 호출됨
	void OnPossess(APawn* NewPawn) override;
	// 클라이언트에서만 호출됨, 리슨서버도.
	void AcknowledgePossession(APawn* NewPawn) override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	// TeamID에 Team Agent 할당
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	// FGenericTeamId 형식으로 TeamID 탐색
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerNotifyLoaded();

	UFUNCTION(Server, Reliable)
	void ServerSetLoadoutColor(const FMaterialParamDataPair& ColorData);

	UFUNCTION(Server, Reliable)
	void ServerSetLoadoutWeaponId(FName WeaponId);

	UFUNCTION(Server, Reliable)
	void ServerSetLoopReady(bool bReady);


private:
	void SpawnGameplayWidget();
	void HandleGameStateChanged(EMAGameState NewState);

	UPROPERTY()
	class AMAPlayerCharacter* MAPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UMAGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	class UMAGameplayWidget* GameplayWidget;

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
	// 마우스 커서 관련 여기 코드는 강의에는 없는 별도 코드입니다.
	bool bOnMouseCursorRecord = false;
	void CheckMouseCursorShape();
	// 여기까지
};
