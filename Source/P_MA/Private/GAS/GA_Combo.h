// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "DebugShapeHelper.h"
#include "GA_Combo.generated.h"

class UMASkillVFXSet;
/**
 * 
 */
UCLASS()
class UGA_Combo : public UMAGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Combo();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	static FGameplayTag GetComboChangeEventTag();
	static FGameplayTag GetComboChangeEventEndTag();
	static FGameplayTag GetComboTargetEventTag();
	static FGameplayTag GetComboClearEventTag();
	
private:
	void SetupWaitComboInputPress();

	UFUNCTION()
	void HandleInputPress(float TimeWaited);
	void TryCommitCombo();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> FuryEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TMap<FName, TSubclassOf<UGameplayEffect>> DamageEffectMap;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	EVA_Shape DamageShape = EVA_Shape::Sphere;
	
	TSubclassOf<UGameplayEffect> GetDamageEffectForCurrentCombo() const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ComboMontage;

	UFUNCTION()
	void ComboChangedEventReceived(FGameplayEventData Data);
	
	UFUNCTION()
	void DoDamage(FGameplayEventData Data);

	UFUNCTION()
	void ClearIgnore(FGameplayEventData Data);

	UFUNCTION()
	void HandleVFXSpawnEvent(FGameplayEventData Payload);
	
	FName NextComboName;

	UPROPERTY()
	TArray<AActor*> IgnoreTargets;

	UPROPERTY(EditAnywhere, Category="VFX")
	TObjectPtr<UMASkillVFXSet> VFXDataSet;
	
	FGameplayTag VFXRootTag;
};
