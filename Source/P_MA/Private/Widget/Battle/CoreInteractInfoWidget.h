// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoreInteractInfoWidget.generated.h"

class UTextBlock;

UCLASS()
class P_MA_API UCoreInteractInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Core")
	void SetInfoText(const FText& InText);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> InfoText;
};
