// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Abilities/GameplayAbility.h"
#include "SkillDragDropOperation.generated.h"

UCLASS()
class USkillDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 드래그 중인 스킬 클래스 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	TSubclassOf<UGameplayAbility> SkillClass;
};