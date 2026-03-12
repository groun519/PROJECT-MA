// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadoutTabWidgetBase.generated.h"

class UWidget;
class UScrollBox;
struct FMaterialParamData;

UCLASS(Abstract)
class P_MA_API ULoadoutTabWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	static void AddButtonToScrollBox(UScrollBox* ScrollBox, UWidget* ButtonWidget, const FMargin& Padding = FMargin(6.f, 0.f, 6.f, 0.f));
	static bool IsSameColorData(const FMaterialParamData& A, const FMaterialParamData& B);
};
