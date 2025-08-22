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
	
	// TeamID에 Team Agent 할당
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	// FGenericTeamId 형식으로 TeamID 탐색
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
private:
	void SpawnHUDWidget();

	UPROPERTY()
	class AMAPlayerCharacter* MAPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UMAHUD> HUDWidgetClass;

	UPROPERTY()
	class UMAHUD* HUDWidget;

	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
};
