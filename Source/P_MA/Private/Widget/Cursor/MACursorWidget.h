// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MACursorWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class P_MA_API UMACursorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetBaseColor(const FLinearColor& InColor);

private:
	void EnsureCursorMaterial();

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	TObjectPtr<UImage> CursorImage;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CursorMID;

	UPROPERTY(EditDefaultsOnly, Category = "Cursor")
	FName BaseColorParamName = TEXT("BaseColor");
};

