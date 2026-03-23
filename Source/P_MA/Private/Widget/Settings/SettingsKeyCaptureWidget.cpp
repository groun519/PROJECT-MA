// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsKeyCaptureWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"

namespace
{
	FText GetPreviewKeyText(const FText& InKeyText)
	{
		return InKeyText.IsEmpty() ? FText::FromString(TEXT("-")) : InKeyText;
	}
}

void USettingsKeyCaptureWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	CloseButton->OnClicked.AddUniqueDynamic(this, &USettingsKeyCaptureWidget::HandleCloseButtonClicked);
}

FReply USettingsKeyCaptureWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnCanceled.Broadcast();
		return FReply::Handled();
	}

	OnKeyCaptured.Broadcast(InKeyEvent.GetKey());
	return FReply::Handled();
}

FReply USettingsKeyCaptureWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnKeyCaptured.Broadcast(InMouseEvent.GetEffectingButton());
	return FReply::Handled();
}

void USettingsKeyCaptureWidget::SetupCaptureDisplay(const FText& InActionName, const FText& InCurrentKeyText)
{
	BindingPreviewText->SetText(InActionName);
	CurrentKeyPreviewText->SetText(GetPreviewKeyText(InCurrentKeyText));
	SetPendingKeyText(NSLOCTEXT("SettingsKeyCaptureWidget", "Listening", "..."));
}

void USettingsKeyCaptureWidget::SetPendingKeyText(const FText& InPendingKeyText)
{
	PendingKeyPreviewText->SetText(GetPreviewKeyText(InPendingKeyText));
	ResetConflictState();
}

void USettingsKeyCaptureWidget::ShowConflictStatus(const FText& InStatusText)
{
	ConflictIndicatorImage->SetColorAndOpacity(FLinearColor(1.f, 0.15f, 0.15f, 1.f));
	StatusText->SetText(InStatusText);
	StatusText->SetVisibility(ESlateVisibility::Visible);
}

void USettingsKeyCaptureWidget::ResetConflictState()
{
	ConflictIndicatorImage->SetColorAndOpacity(FLinearColor::White);
	StatusText->SetText(FText::FromString(TEXT(" ")));
	StatusText->SetVisibility(ESlateVisibility::Visible);
}

void USettingsKeyCaptureWidget::HandleCloseButtonClicked()
{
	OnCanceled.Broadcast();
}
