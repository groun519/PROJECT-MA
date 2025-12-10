// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAMainMenuWidget.h"
#include "Components/Button.h"
#include "kismet/GameplayStatics.h"

void UMAMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼이 유효한지 확인 후 클릭 이벤트 연결
	if (StartGameButton)
	{
		// 이전에 연결된게 있다면 끊고 다시 연결 (중복 방지 안전장치)
		StartGameButton->OnClicked.RemoveDynamic(this, &UMAMainMenuWidget::OnStartGameClicked);
		StartGameButton->OnClicked.AddDynamic(this, &UMAMainMenuWidget::OnStartGameClicked);
	}
}

void UMAMainMenuWidget::OnStartGameClicked()
{
	// 레벨 이름이 비어있지 않다면 이동
	if (!LevelToLoad.IsNone())
	{
		UGameplayStatics::OpenLevel(this, LevelToLoad);
	}
}