// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MAGameStateTypes.generated.h"

UENUM(BlueprintType)
enum class EMAGameState : uint8
{
	// 멈춤
	Wait = 0,
	// 게임 시작 단계. 루프 탈출 시에도 사용.
	Start = 1,

	/** Inf Loop **/

	// 전투 섹터 진입
	InBattle = 2,
	// 전투 중(웨이브)
	Battle = 3,
	// 전투 완료 후 모두 플랫폼에 올라타기 전까지 대기 단계
	EndBattle = 4,
	// 전투 섹터 빠져나가고 루프 진입까지
	OutBattle = 5,
	// 스플라인 섹터 무한반복하며 정비
	Loop = 6,
};
