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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = "true"))
	TSubclassOf<UGameplayAbility> SkillClass;
};