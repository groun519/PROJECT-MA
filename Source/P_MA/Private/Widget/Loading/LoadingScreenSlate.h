// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UMAGameInstance;

class SLoadingScreenRoot : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingScreenRoot) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UMAGameInstance>, GameInstance)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TWeakObjectPtr<UMAGameInstance> GameInstance;
	float DisplayProgress = 0.0f;
	double LastUpdateSeconds = 0.0;
	double FirstSeenSeconds = 0.0;
	bool bFinishPhase = false;
	double FinishStartSeconds = 0.0;

	TOptional<float> GetProgress();
	FText GetPercentText();
};
