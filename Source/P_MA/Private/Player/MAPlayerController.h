// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MAPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AMAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// 서버에서만 호출됨
	void OnPossess(APawn* NewPawn) override;
	// 클라이언트에서만 호출됨, 리슨서버도.
	void AcknowledgePossession(APawn* NewPawn) override;
	
private:
	void SpawnHUDWidget();

	UPROPERTY()
	class AMAPlayerCharacter* MAPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UHUDWidget> HUDWidgetClass;

	UPROPERTY()
	class UHUDWidget* HUDWidget;
};
