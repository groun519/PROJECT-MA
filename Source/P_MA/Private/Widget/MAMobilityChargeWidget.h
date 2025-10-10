// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "MAMobilityChargeWidget.generated.h"

/**
 * 
 */
UCLASS()
class UMAMobilityChargeWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ChargeProgressBar;

private:
	UFUNCTION()
	void ShowChargeBar();

	UFUNCTION()
	void UpdateChargeBar(float ChargePercentage);

	UFUNCTION()
	void HideChargeBar();
	
};
