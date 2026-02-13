// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Loop/LoopPlayerStatusWidget.h"
#include "LoopReadyWidget.generated.h"

class UButton;
class UPanelWidget;

UCLASS()
class P_MA_API ULoopReadyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PlayerStatusBox;

	UPROPERTY(EditDefaultsOnly, Category = "LoopReady")
	TSubclassOf<ULoopPlayerStatusWidget> PlayerStatusWidgetClass;

	void UpdatePlayerStatuses(const TArray<FLoopReadyPlayerStatus>& Statuses);
	void RefreshFromGameState();

private:
	UFUNCTION()
	void HandleReadyClicked();

	void HandleLoopReadyEntriesChanged();
	void EnsureEntryWidgets(int32 Count);

	UPROPERTY()
	TArray<TObjectPtr<ULoopPlayerStatusWidget>> StatusWidgets;
};
