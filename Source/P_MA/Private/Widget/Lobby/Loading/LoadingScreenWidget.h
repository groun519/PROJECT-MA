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
class UImage;
class ULoadingBackgroundData;
class ULoadingTooltipData;

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
	void UpdateLoadingProgress(
		float TargetProgress,
		bool bLoadingComplete,
		float InFinishDurationSeconds,
		float InWarmupDurationSeconds,
		float InWarmupMax,
		float InMainMax
	);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoadingPercentText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PlayerStatusBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LoadingBackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LoadingTooltipText;

	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSubclassOf<ULoadingPlayerStatusWidget> PlayerStatusWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TObjectPtr<ULoadingBackgroundData> BackgroundData;

	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TObjectPtr<ULoadingTooltipData> TooltipData;

private:
	void EnsureEntryWidgets(int32 Count);
	void ApplyProgressFromTarget(float TargetProgress, bool bLoadingComplete);

	float DisplayProgress = 0.0f;
	double LastUpdateSeconds = 0.0;
	double FirstSeenSeconds = 0.0;
	bool bFinishPhase = false;
	double FinishStartSeconds = 0.0;
	float PendingTargetProgress = 0.0f;
	bool bPendingLoadingComplete = false;
	bool bHasPendingProgress = false;

	float WarmupDurationSeconds = 3.0f;
	float WarmupMax = 0.50f;
	float MainMax = 0.95f;
	float FinishDurationSeconds = 1.0f;

	UPROPERTY()
	TArray<TObjectPtr<ULoadingPlayerStatusWidget>> StatusWidgets;
};
