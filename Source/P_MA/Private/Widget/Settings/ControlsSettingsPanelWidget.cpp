// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/ControlsSettingsPanelWidget.h"

#include "Components/VerticalBox.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "Input/MAInputStatics.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Player/MAPlayerController.h"
#include "Player/MAPlayerControllerBase.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Widget/Settings/SettingsKeyBindingRowWidget.h"
#include "Widget/Settings/SettingsKeyCaptureWidget.h"
#include "Widget/Settings/SettingsSectionHeaderWidget.h"

namespace
{
	struct FBindingRowData
	{
		FName MappingName;
		FText DisplayName;
		FText DisplayCategory;
		FText PrimaryKeyText;
		FText SecondaryKeyText;
		bool bHasSecondaryKey = false;
	};

	FText GetBindingDisplayName(const FEnhancedActionKeyMapping& Mapping)
	{
		return Mapping.GetDisplayName().IsEmpty() ? FText::FromName(Mapping.GetMappingName()) : Mapping.GetDisplayName();
	}

	FText GetBindingKeyText(const FKey& Key)
	{
		return Key.IsValid() ? FMAInputStatics::GetKeyDisplayText(Key) : FText::GetEmpty();
	}

	FText GetListeningKeyText()
	{
		return NSLOCTEXT("ControlsSettingsPanel", "PressAnyKey", "...");
	}

	FText GetBindingConflictMessage(const FText& KeyText, const FText& ConflictDisplayName)
	{
		return FText::Format(
			NSLOCTEXT("ControlsSettingsPanel", "BindingConflict", "{0} is already used for {1}."),
			KeyText,
			ConflictDisplayName);
	}

	FText GetDisallowedBindingKeyMessage()
	{
		return NSLOCTEXT("ControlsSettingsPanel", "DisallowedBindingKey", "This key cannot be assigned.");
	}
}

void UControlsSettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RegisterSourceContexts();
	RebuildBindingRows();
}

void UControlsSettingsPanelWidget::NativeDestruct()
{
	StopListeningForBinding(false);
	Super::NativeDestruct();
}

/** Row Generation **/
const TArray<TObjectPtr<UInputMappingContext>>* UControlsSettingsPanelWidget::GetEffectiveSourceContexts() const
{
	const UControlsSettingsPanelWidget* DefaultWidget = GetClass()->GetDefaultObject<UControlsSettingsPanelWidget>();
	return SourceContexts.Num() > 0 ? &SourceContexts : (DefaultWidget ? &DefaultWidget->SourceContexts : nullptr);
}

TSubclassOf<USettingsKeyBindingRowWidget> UControlsSettingsPanelWidget::GetEffectiveKeyBindingRowClass() const
{
	const UControlsSettingsPanelWidget* DefaultWidget = GetClass()->GetDefaultObject<UControlsSettingsPanelWidget>();
	return KeyBindingRowClass ? KeyBindingRowClass : (DefaultWidget ? DefaultWidget->KeyBindingRowClass : nullptr);
}

TSubclassOf<USettingsSectionHeaderWidget> UControlsSettingsPanelWidget::GetEffectiveCategoryHeaderWidgetClass() const
{
	const UControlsSettingsPanelWidget* DefaultWidget = GetClass()->GetDefaultObject<UControlsSettingsPanelWidget>();
	return CategoryHeaderWidgetClass ? CategoryHeaderWidgetClass : (DefaultWidget ? DefaultWidget->CategoryHeaderWidgetClass : nullptr);
}

TSubclassOf<USettingsKeyCaptureWidget> UControlsSettingsPanelWidget::GetEffectiveKeyCaptureWidgetClass() const
{
	const UControlsSettingsPanelWidget* DefaultWidget = GetClass()->GetDefaultObject<UControlsSettingsPanelWidget>();
	return KeyCaptureWidgetClass ? KeyCaptureWidgetClass : (DefaultWidget ? DefaultWidget->KeyCaptureWidgetClass : nullptr);
}

void UControlsSettingsPanelWidget::RegisterSourceContexts()
{
	const TArray<TObjectPtr<UInputMappingContext>>* EffectiveSourceContexts = GetEffectiveSourceContexts();
	if (!EffectiveSourceContexts) return;

	for (const UInputMappingContext* Context : *EffectiveSourceContexts)
	{
		FMAInputStatics::RegisterInputMappingContextDefaults(GetOwningPlayer(), Context);
	}
}

void UControlsSettingsPanelWidget::RebuildBindingRows()
{
	const TSubclassOf<USettingsKeyBindingRowWidget> EffectiveKeyBindingRowClass = GetEffectiveKeyBindingRowClass();
	const TSubclassOf<USettingsSectionHeaderWidget> EffectiveCategoryHeaderWidgetClass = GetEffectiveCategoryHeaderWidgetClass();
	const TArray<TObjectPtr<UInputMappingContext>>* EffectiveSourceContexts = GetEffectiveSourceContexts();

	if (!BindingRowsBox || !EffectiveKeyBindingRowClass || !GetOwningPlayer() || !EffectiveSourceContexts) return;

	BindingRowsBox->ClearChildren();
	RowMetadataByWidget.Empty();
	FString LastCategoryName;
	TArray<FBindingRowData> BindingRows;
	TMap<FName, int32> BindingRowIndexByName;
	UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();

	for (const UInputMappingContext* Context : *EffectiveSourceContexts)
	{
		if (!Context) continue;

		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (!Mapping.IsPlayerMappable()) continue;

			const FName MappingName = Mapping.GetMappingName();
			if (MappingName.IsNone()) continue;
			if (int32* ExistingRowIndex = BindingRowIndexByName.Find(MappingName))
			{
				FBindingRowData& ExistingRow = BindingRows[*ExistingRowIndex];
				if (!ExistingRow.bHasSecondaryKey)
				{
					ExistingRow.SecondaryKeyText = GetCurrentKeyText(UserSettings, MappingName, 1, Mapping.Key);
					ExistingRow.bHasSecondaryKey = true;
				}
				continue;
			}

			FBindingRowData& NewRow = BindingRows.AddDefaulted_GetRef();
			NewRow.MappingName = MappingName;
			NewRow.DisplayName = GetBindingDisplayName(Mapping);
			NewRow.DisplayCategory = Mapping.GetDisplayCategory();
			NewRow.PrimaryKeyText = GetCurrentKeyText(UserSettings, MappingName, 0, Mapping.Key);
			BindingRowIndexByName.Add(MappingName, BindingRows.Num() - 1);
		}
	}

	BindingRows.Sort([](const FBindingRowData& A, const FBindingRowData& B)
	{
		const FString ACategory = A.DisplayCategory.ToString();
		const FString BCategory = B.DisplayCategory.ToString();
		if (ACategory != BCategory)
		{
			if (ACategory.IsEmpty()) return false;
			if (BCategory.IsEmpty()) return true;
			return ACategory < BCategory;
		}

		return A.DisplayName.ToString() < B.DisplayName.ToString();
	});

	for (const FBindingRowData& BindingRow : BindingRows)
	{
		const FText DisplayCategory = BindingRow.DisplayCategory;
		const FString DisplayCategoryString = DisplayCategory.ToString();
		if (EffectiveCategoryHeaderWidgetClass && !DisplayCategoryString.IsEmpty() && DisplayCategoryString != LastCategoryName)
		{
			USettingsSectionHeaderWidget* HeaderRow = CreateWidget<USettingsSectionHeaderWidget>(GetOwningPlayer(), EffectiveCategoryHeaderWidgetClass);
			if (HeaderRow)
			{
				HeaderRow->SetupHeader(DisplayCategory);
				BindingRowsBox->AddChild(HeaderRow);
			}
			LastCategoryName = DisplayCategoryString;
		}

		USettingsKeyBindingRowWidget* Row = CreateWidget<USettingsKeyBindingRowWidget>(GetOwningPlayer(), EffectiveKeyBindingRowClass);
		if (!Row) continue;

		Row->SetupRow(BindingRow.DisplayName, BindingRow.PrimaryKeyText);
		if (BindingRow.bHasSecondaryKey) Row->SetupSecondaryKey(BindingRow.SecondaryKeyText);
		Row->OnRebindRequested.AddUObject(this, &UControlsSettingsPanelWidget::HandleRowRebindRequested);
		Row->OnResetRequested.AddUObject(this, &UControlsSettingsPanelWidget::HandleRowResetRequested);

		FBindingRowMeta RowMeta;
		RowMeta.MappingName = BindingRow.MappingName;
		RowMeta.DisplayName = BindingRow.DisplayName;
		RowMetadataByWidget.Add(Row, RowMeta);
		BindingRowsBox->AddChild(Row);
	}
}

/** Binding Actions **/
UEnhancedInputUserSettings* UControlsSettingsPanelWidget::GetEnhancedInputUserSettings() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			return InputSubsystem->GetUserSettings();
		}
	}

	return nullptr;
}

void UControlsSettingsPanelWidget::HandleRowRebindRequested(USettingsKeyBindingRowWidget* Row, int32 SlotIndex)
{
	if (!Row) return;

	const FBindingRowMeta* RowMeta = RowMetadataByWidget.Find(Row);
	if (!RowMeta || RowMeta->MappingName.IsNone()) return;

	BeginListeningForBinding(Row, RowMeta->MappingName, RowMeta->DisplayName, SlotIndex);
}

void UControlsSettingsPanelWidget::HandleRowResetRequested(USettingsKeyBindingRowWidget* Row)
{
	if (!Row) return;

	const FBindingRowMeta* RowMeta = RowMetadataByWidget.Find(Row);
	if (!RowMeta || RowMeta->MappingName.IsNone()) return;

	CancelListeningForBinding();
	if (ResetBindingRow(RowMeta->MappingName)) RebuildBindingRows();
}

bool UControlsSettingsPanelWidget::IsDisallowedBindingKey(const FKey& Key) const
{
	return Key == EKeys::Escape
		|| Key == EKeys::MouseWheelAxis
		|| Key == EKeys::MouseX
		|| Key == EKeys::MouseY
		|| Key == EKeys::Pause
		|| Key == EKeys::ScrollLock
		|| Key == EKeys::MouseScrollUp
		|| Key == EKeys::MouseScrollDown;
}

bool UControlsSettingsPanelWidget::FindBindingConflict(const FKey& NewKey, const FName IgnoredMappingName, int32 IgnoredSlotIndex, FText& OutConflictDisplayName) const
{
	if (!NewKey.IsValid()) return false;

	UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!UserSettings) return false;

	const TArray<TObjectPtr<UInputMappingContext>>* EffectiveSourceContexts = GetEffectiveSourceContexts();
	if (!EffectiveSourceContexts) return false;

	TMap<FName, int32> MappingSlotCounts;

	for (const UInputMappingContext* Context : *EffectiveSourceContexts)
	{
		if (!Context) continue;

		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (!Mapping.IsPlayerMappable()) continue;

			const FName MappingName = Mapping.GetMappingName();
			if (MappingName.IsNone()) continue;

			int32& SlotCount = MappingSlotCounts.FindOrAdd(MappingName);
			const int32 SlotIndex = SlotCount;
			++SlotCount;

			if (MappingName == IgnoredMappingName && SlotIndex == IgnoredSlotIndex) continue;
			if (GetCurrentKey(UserSettings, MappingName, SlotIndex, Mapping.Key) != NewKey) continue;

			OutConflictDisplayName = GetBindingDisplayName(Mapping);
			return true;
		}
	}

	return false;
}

FKey UControlsSettingsPanelWidget::GetCurrentKey(UEnhancedInputUserSettings* UserSettings, const FName MappingName, int32 SlotIndex, const FKey& DefaultKey) const
{
	if (UserSettings)
	{
		if (const FPlayerKeyMapping* Mapping = UserSettings->FindCurrentMappingForSlot(MappingName, GetKeySlotByIndex(SlotIndex)))
		{
			return Mapping->GetCurrentKey();
		}
	}

	return DefaultKey;
}

bool UControlsSettingsPanelWidget::ApplyPendingBinding(const FKey& NewKey)
{
	if (!PendingBindingTarget.IsValid() || !NewKey.IsValid()) return false;

	UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!UserSettings)
	{
		CancelListeningForBinding();
		return false;
	}

	FMapPlayerKeyArgs Args;
	Args.MappingName = PendingBindingTarget.MappingName;
	Args.Slot = GetKeySlotByIndex(PendingBindingTarget.SlotIndex);
	Args.NewKey = NewKey;

	FGameplayTagContainer FailureReason;
	UserSettings->MapPlayerKey(Args, FailureReason);
	if (!FailureReason.IsEmpty())
	{
		CancelListeningForBinding();
		RebuildBindingRows();
		return false;
	}

	ApplyAndSaveInputSettings(UserSettings);
	CancelListeningForBinding();
	RebuildBindingRows();
	return true;
}

bool UControlsSettingsPanelWidget::ResetBindingRow(const FName MappingName)
{
	UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings();
	if (!UserSettings) return false;

	FMapPlayerKeyArgs Args;
	Args.MappingName = MappingName;

	FGameplayTagContainer FailureReason;
	UserSettings->ResetAllPlayerKeysInRow(Args, FailureReason);
	if (!FailureReason.IsEmpty()) return false;

	ApplyAndSaveInputSettings(UserSettings);
	return true;
}

FText UControlsSettingsPanelWidget::GetCurrentKeyText(UEnhancedInputUserSettings* UserSettings, const FName MappingName, int32 SlotIndex, const FKey& DefaultKey) const
{
	return GetBindingKeyText(GetCurrentKey(UserSettings, MappingName, SlotIndex, DefaultKey));
}

void UControlsSettingsPanelWidget::ApplyAndSaveInputSettings(UEnhancedInputUserSettings* UserSettings) const
{
	UserSettings->ApplySettings();
	UserSettings->AsyncSaveSettings();

	if (AMAPlayerController* MAPlayerController = GetOwningPlayer<AMAPlayerController>())
	{
		MAPlayerController->NotifyInputBindingsChanged();
	}
}

EPlayerMappableKeySlot UControlsSettingsPanelWidget::GetKeySlotByIndex(int32 SlotIndex)
{
	return SlotIndex == 1 ? EPlayerMappableKeySlot::Second : EPlayerMappableKeySlot::First;
}

/** Key Capture **/
void UControlsSettingsPanelWidget::BeginListeningForBinding(USettingsKeyBindingRowWidget* Row, const FName MappingName, const FText& DisplayName, int32 SlotIndex)
{
	CancelListeningForBinding();

	const TSubclassOf<USettingsKeyCaptureWidget> EffectiveKeyCaptureWidgetClass = GetEffectiveKeyCaptureWidgetClass();
	if (!EffectiveKeyCaptureWidgetClass || !GetOwningPlayer()) return;

	ActiveKeyCaptureWidget = CreateWidget<USettingsKeyCaptureWidget>(GetOwningPlayer(), EffectiveKeyCaptureWidgetClass);
	if (!ActiveKeyCaptureWidget) return;

	PendingBindingTarget.Row = Row;
	PendingBindingTarget.MappingName = MappingName;
	PendingBindingTarget.DisplayName = DisplayName;
	PendingBindingTarget.SlotIndex = SlotIndex;

	const FText CurrentKeyText = GetCurrentKeyText(GetEnhancedInputUserSettings(), MappingName, SlotIndex, EKeys::Invalid);
	Row->SetKeyTextBySlot(SlotIndex, GetListeningKeyText());
	ActiveKeyCaptureWidget->SetupCaptureDisplay(DisplayName, CurrentKeyText);
	ActiveKeyCaptureWidget->OnKeyCaptured.AddUObject(this, &UControlsSettingsPanelWidget::HandleKeyCaptured);
	ActiveKeyCaptureWidget->OnCanceled.AddUObject(this, &UControlsSettingsPanelWidget::HandleKeyCaptureCanceled);
	ActiveKeyCaptureWidget->AddToViewport(220);

	if (AMAPlayerControllerBase* MAPlayerController = GetOwningPlayer<AMAPlayerControllerBase>())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ActiveKeyCaptureWidget->TakeWidget());
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		MAPlayerController->SetInputMode(InputMode);
		MAPlayerController->bShowMouseCursor = true;
	}

	ActiveKeyCaptureWidget->SetFocus();
}

void UControlsSettingsPanelWidget::HandleKeyCaptured(const FKey& CapturedKey)
{
	if (ActiveKeyCaptureWidget) ActiveKeyCaptureWidget->SetPendingKeyText(GetBindingKeyText(CapturedKey));

	if (IsDisallowedBindingKey(CapturedKey))
	{
		if (ActiveKeyCaptureWidget)
		{
			ActiveKeyCaptureWidget->ShowConflictStatus(GetDisallowedBindingKeyMessage());
		}
		return;
	}

	FText ConflictDisplayName;
	if (FindBindingConflict(CapturedKey, PendingBindingTarget.MappingName, PendingBindingTarget.SlotIndex, ConflictDisplayName))
	{
		if (ActiveKeyCaptureWidget)
		{
			ActiveKeyCaptureWidget->ShowConflictStatus(GetBindingConflictMessage(GetBindingKeyText(CapturedKey), ConflictDisplayName));
		}
		return;
	}

	ApplyPendingBinding(CapturedKey);
}

void UControlsSettingsPanelWidget::HandleKeyCaptureCanceled()
{
	CancelListeningForBinding();
}

void UControlsSettingsPanelWidget::CancelListeningForBinding()
{
	StopListeningForBinding(true);
}

void UControlsSettingsPanelWidget::StopListeningForBinding(bool bRestoreFocus)
{
	RestorePendingBindingRow();
	CloseKeyCaptureWidget(bRestoreFocus);
	PendingBindingTarget = {};
}

void UControlsSettingsPanelWidget::CloseKeyCaptureWidget(bool bRestoreFocus)
{
	if (!ActiveKeyCaptureWidget) return;

	ActiveKeyCaptureWidget->RemoveFromParent();
	ActiveKeyCaptureWidget = nullptr;

	if (bRestoreFocus)
	{
		if (AMAPlayerControllerBase* MAPlayerController = GetOwningPlayer<AMAPlayerControllerBase>())
		{
			MAPlayerController->RefreshSettingsFocus();
		}
	}
}

void UControlsSettingsPanelWidget::RestorePendingBindingRow()
{
	if (!PendingBindingTarget.IsValid()) return;

	if (UEnhancedInputUserSettings* UserSettings = GetEnhancedInputUserSettings())
	{
		PendingBindingTarget.Row->SetKeyTextBySlot(
			PendingBindingTarget.SlotIndex,
			GetCurrentKeyText(UserSettings, PendingBindingTarget.MappingName, PendingBindingTarget.SlotIndex, EKeys::Invalid));
		return;
	}

	PendingBindingTarget.Row->SetKeyTextBySlot(PendingBindingTarget.SlotIndex, FText::GetEmpty());
}
