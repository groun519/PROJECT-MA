// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "SkillBehavior_Default.generated.h"

/**
 * 
 */
UCLASS()
class USkillBehavior_Default : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
};
