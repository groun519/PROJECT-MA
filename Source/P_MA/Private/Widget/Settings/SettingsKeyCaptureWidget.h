// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsKeyCaptureWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSettingsKeyCaptured, const FKey&);
DECLARE_MULTICAST_DELEGATE(FOnSettingsKeyCaptureCanceled);

UCLASS()
class P_MA_API USettingsKeyCaptureWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void SetupCaptureDisplay(const FText& InActionName, const FText& InCurrentKeyText);
	void SetPendingKeyText(const FText& InPendingKeyText);
	void ShowConflictStatus(const FText& InStatusText);

	FOnSettingsKeyCaptured OnKeyCaptured;
	FOnSettingsKeyCaptureCanceled OnCanceled;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BindingPreviewText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentKeyPreviewText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PendingKeyPreviewText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ConflictIndicatorImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

private:
	void ResetConflictState();

	UFUNCTION()
	void HandleCloseButtonClicked();
};
