// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Settings/SettingsPanelWidgetBase.h"
#include "ControlsSettingsPanelWidget.generated.h"

enum class EPlayerMappableKeySlot : uint8;
class UInputMappingContext;
class USettingsKeyCaptureWidget;
class USettingsKeyBindingRowWidget;
class USettingsSectionHeaderWidget;
class UEnhancedInputUserSettings;
class UVerticalBox;

UCLASS()
class P_MA_API UControlsSettingsPanelWidget : public USettingsPanelWidgetBase
{
	GENERATED_BODY()

public:
	/** Lifecycle **/
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	struct FPendingBindingTarget
	{
		TObjectPtr<USettingsKeyBindingRowWidget> Row;
		FName MappingName;
		FText DisplayName;
		int32 SlotIndex = INDEX_NONE;

		bool IsValid() const { return Row && MappingName.IsValid() && SlotIndex != INDEX_NONE; }
	};

	struct FBindingRowMeta
	{
		FName MappingName;
		FText DisplayName;
	};

	/** Row Generation **/
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TObjectPtr<UInputMappingContext>> SourceContexts;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSubclassOf<USettingsKeyBindingRowWidget> KeyBindingRowClass;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSubclassOf<USettingsSectionHeaderWidget> CategoryHeaderWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSubclassOf<USettingsKeyCaptureWidget> KeyCaptureWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> BindingRowsBox;

	/** Key Capture **/
	UPROPERTY(Transient)
	TObjectPtr<USettingsKeyCaptureWidget> ActiveKeyCaptureWidget;

	TMap<TObjectPtr<USettingsKeyBindingRowWidget>, FBindingRowMeta> RowMetadataByWidget;
	FPendingBindingTarget PendingBindingTarget;

	/** Row Generation **/
	const TArray<TObjectPtr<UInputMappingContext>>* GetEffectiveSourceContexts() const;
	TSubclassOf<USettingsKeyBindingRowWidget> GetEffectiveKeyBindingRowClass() const;
	TSubclassOf<USettingsSectionHeaderWidget> GetEffectiveCategoryHeaderWidgetClass() const;
	TSubclassOf<USettingsKeyCaptureWidget> GetEffectiveKeyCaptureWidgetClass() const;
	void RegisterSourceContexts();
	void RebuildBindingRows();

	/** Binding Actions **/
	UEnhancedInputUserSettings* GetEnhancedInputUserSettings() const;
	void HandleRowRebindRequested(USettingsKeyBindingRowWidget* Row, int32 SlotIndex);
	void HandleRowResetRequested(USettingsKeyBindingRowWidget* Row);
	bool ApplyPendingBinding(const FKey& NewKey);
	bool ResetBindingRow(const FName MappingName);
	bool IsDisallowedBindingKey(const FKey& Key) const;
	bool FindBindingConflict(const FKey& NewKey, const FName IgnoredMappingName, int32 IgnoredSlotIndex, FText& OutConflictDisplayName) const;
	FKey GetCurrentKey(UEnhancedInputUserSettings* UserSettings, const FName MappingName, int32 SlotIndex, const FKey& DefaultKey) const;
	FText GetCurrentKeyText(UEnhancedInputUserSettings* UserSettings, const FName MappingName, int32 SlotIndex, const FKey& DefaultKey) const;
	void ApplyAndSaveInputSettings(UEnhancedInputUserSettings* UserSettings) const;
	static EPlayerMappableKeySlot GetKeySlotByIndex(int32 SlotIndex);

	/** Key Capture **/
	void BeginListeningForBinding(USettingsKeyBindingRowWidget* Row, const FName MappingName, const FText& DisplayName, int32 SlotIndex);
	void HandleKeyCaptured(const FKey& CapturedKey);
	void HandleKeyCaptureCanceled();
	void CancelListeningForBinding();
	void StopListeningForBinding(bool bRestoreFocus);
	void CloseKeyCaptureWidget(bool bRestoreFocus);
	void RestorePendingBindingRow();
};
