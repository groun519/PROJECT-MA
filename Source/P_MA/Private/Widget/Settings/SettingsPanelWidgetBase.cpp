// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsPanelWidgetBase.h"

#include "Components/Button.h"

/** Lifecycle **/
void USettingsPanelWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	DefaultButton->OnClicked.AddUniqueDynamic(this, &USettingsPanelWidgetBase::HandleDefaultRequested);
}
