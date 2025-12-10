// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAMainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class UMAMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 위젯이 생성되고 화면에 그려질 때 호출됩니다 (BeginPlay와 유사)
	virtual void NativeConstruct() override;

	// 버튼 클릭 시 호출될 함수
	UFUNCTION()
	void OnStartGameClicked();

private:
	// [중요] 메타 속성 BindWidget을 쓰면, 에디터의 버튼 이름과 이 변수 이름이 같아야 연결됩니다.
	// UMG 에디터에서 버튼 이름을 반드시 'StartGameButton'으로 지어야 합니다.
	UPROPERTY(meta = (BindWidget))
	class UButton* StartGameButton;

public:
	// 이동할 레벨 이름을 에디터에서 설정 가능하게 뺍니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	FName LevelToLoad = "GameLevel"; // 기본값 설정
	
};
