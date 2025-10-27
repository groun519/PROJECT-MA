// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_ApplyEffectTarget.generated.h"

/**
 * 지점 즉시 효과 적용
 * 플레이어가 지정한 위치(Target Point)를 받아 그 위치를 중심으로 범위 내의 적에게 즉시 데미지
 */
UCLASS()
class USkillBehavior_ApplyEffectTarget : public UMASkillBehavior
{
	GENERATED_BODY()
	
};
