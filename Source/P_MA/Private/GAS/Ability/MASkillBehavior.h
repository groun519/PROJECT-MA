// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MASkillBehavior.generated.h"

class UMAGameplayAbility_SkillBase;
class AMAPlayerCharacter;
/**
 * 
 */
UCLASS(Blueprintable, Abstract, EditInlineNew)
class UMASkillBehavior : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UMAGameplayAbility_SkillBase> OwningAbility;

	UFUNCTION(BlueprintNativeEvent, Category="Skill Behavior")
	void OnActivate();
	virtual void OnActivate_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category="Skill Behavior")
	void OnEndAbility();
	virtual void OnEndAbility_Implementation();

	UPROPERTY()
	TObjectPtr<AMAPlayerCharacter> PlayerCharacter;

	class AMAPlayerCharacter* GetPlayerCharacter();
};
