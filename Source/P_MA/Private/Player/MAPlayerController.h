// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
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

	virtual void Tick(float DeltaSeconds) override;
	
	// TeamID에 Team Agent 할당
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	// FGenericTeamId 형식으로 TeamID 탐색
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

private:
	void SpawnGameplayWidget();

	UPROPERTY()
	class AMAPlayerCharacter* MAPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UMAGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	class UMAGameplayWidget* GameplayWidget;
	
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;

	// 마우스 커서 관련
	bool bOnMouseCursorRecord = false;
	void CheckMouseCursorShape();
};
