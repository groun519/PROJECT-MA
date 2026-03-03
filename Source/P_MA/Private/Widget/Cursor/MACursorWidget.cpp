// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Cursor/MACursorWidget.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UMACursorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureCursorMaterial();
}

void UMACursorWidget::SetBaseColor(const FLinearColor& InColor)
{
	EnsureCursorMaterial();
	if (!CursorMID) return;

	CursorMID->SetVectorParameterValue(BaseColorParamName, InColor);
}

void UMACursorWidget::EnsureCursorMaterial()
{
	if (CursorMID || !CursorImage) return;

	CursorMID = CursorImage->GetDynamicMaterial();
}

