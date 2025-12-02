// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PA_AbilitySystemGenerics.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UUtilityModule;
/**
 * 
 */
UCLASS()
class UPA_AbilitySystemGenerics : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStatEffect() const { return FullStatEffect; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const { return DeathEffect; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDamageEffect() const { return DamageGEClass; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetCooldownEffect() const { return CooldownGEClass; }
	FORCEINLINE const TArray<TSubclassOf<UGameplayEffect>>& GetInitialEffects() const { return InitialEffects; }
	FORCEINLINE const TArray<TSubclassOf<UGameplayAbility>>& GetPassiveAbilities() const { return PassiveAbilities; }
	FORCEINLINE const UDataTable* GetPlayerBaseStatDataTable() const { return PlayerBaseStatDataTable; }
	FORCEINLINE const UDataTable* GetMonsterBaseStatDataTable() const { return MonsterBaseStatDataTable; }
	
	UUtilityModule* FindSkillUtilityModuleByTag(const FGameplayTag& UtilityTag, UObject* Outer) const;
	FORCEINLINE const UDataTable* GetElementDataTable() const {return ElementModuleDataTable;}
	FORCEINLINE const UDataTable* GetBehaviorDataTable() const {return BehaviorModuleDataTable;}
	FORCEINLINE const UDataTable* GetSkillInformationTableTable() const { return SkillInformationDT; }

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> DamageGEClass;
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> CooldownGEClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Base Stats")
	UDataTable* PlayerBaseStatDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "Base Stats")
	UDataTable* MonsterBaseStatDataTable;
	
	UPROPERTY(EditDefaultsOnly, Category="Module")
	UDataTable* UtilityModuleDataTable;
	UPROPERTY(EditDefaultsOnly, Category="Module")
	TObjectPtr<UDataTable> ElementModuleDataTable;
	UPROPERTY(EditDefaultsOnly, Category="Module")
	TObjectPtr<UDataTable> BehaviorModuleDataTable;
	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TObjectPtr<UDataTable> SkillInformationDT;
};