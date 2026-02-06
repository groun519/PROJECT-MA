// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadingScreenWidget.generated.h"

class UPanelWidget;
class ULoadingPlayerStatusWidget;
class UProgressBar;
class UTextBlock;

USTRUCT(BlueprintType)
struct FLoadingPlayerStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	bool bLoaded = false;

	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	FLinearColor BodyColor = FLinearColor::Black;

	UPROPERTY(BlueprintReadOnly, Category = "Loading")
	FLinearColor EyeColor = FLinearColor::White;
};

UCLASS()
class P_MA_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateLoadingStatus(const TArray<FLoadingPlayerStatus>& Statuses);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoadingPercentText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PlayerStatusBox;

	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSubclassOf<ULoadingPlayerStatusWidget> PlayerStatusWidgetClass;

private:
	void EnsureEntryWidgets(int32 Count);

	UPROPERTY()
	TArray<TObjectPtr<ULoadingPlayerStatusWidget>> StatusWidgets;
};
