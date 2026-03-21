// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/GameplaySettingsPanelWidget.h"

#include "Framework/MAGameInstance.h"
#include "Player/MAPlayerControllerBase.h"
#include "Widget/Settings/SettingsDropdownRowWidget.h"

namespace
{
	const TArray<FText>& GetLanguageOptions()
	{
		static const TArray<FText> LanguageOptions =
		{
			NSLOCTEXT("GameplaySettingsPanel", "LanguageKorean", "한국어"),
			NSLOCTEXT("GameplaySettingsPanel", "LanguageEnglish", "English")
		};

		return LanguageOptions;
	}

	FString GetLanguageCultureByIndex(int32 InIndex)
	{
		return InIndex == 0 ? TEXT("ko") : TEXT("en");
	}

	int32 GetLanguageIndexByCulture(const FString& InCulture)
	{
		return InCulture.StartsWith(TEXT("ko")) ? 0 : 1;
	}
}

void UGameplaySettingsPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	LanguageDropdownRow->OnSelectionChanged.AddUObject(this, &UGameplaySettingsPanelWidget::HandleLanguageSelectionChanged);
}

void UGameplaySettingsPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitLanguageRow();
}

void UGameplaySettingsPanelWidget::InitLanguageRow()
{
	const UMAGameInstance* GameInstance = Cast<UMAGameInstance>(GetGameInstance());
	const int32 SelectedLanguageIndex = GetLanguageIndexByCulture(GameInstance ? GameInstance->GetCurrentLanguageCulture() : TEXT("en"));

	LanguageDropdownRow->SetupOptions(
		NSLOCTEXT("GameplaySettingsPanel", "Language", "Language"),
		GetLanguageOptions(),
		SelectedLanguageIndex
	);
}

void UGameplaySettingsPanelWidget::HandleLanguageSelectionChanged(int32 InIndex)
{
	UMAGameInstance* GameInstance = Cast<UMAGameInstance>(GetGameInstance());
	if (!GameInstance) return;

	GameInstance->SetCurrentLanguageCulture(GetLanguageCultureByIndex(InIndex));
	if (AMAPlayerControllerBase* PlayerController = GetOwningPlayer<AMAPlayerControllerBase>())
	{
		PlayerController->ReopenSettingsWidget();
	}
}
