#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Step/MASkillStep_Timed.h"
#include "MASkillStep_Charge.generated.h"

class UAbilityTask_WaitInputRelease;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories="Step")
class P_MA_API UMASkillStep_Charge : public UMASkillStep_Timed
{
	GENERATED_BODY()

public:
	UMASkillStep_Charge();

private:
	virtual float GetStepDuration() const override { return ChargeDuration; }
	virtual void OnTimedStepStarted(UMASkillAbility* SkillAbility, EMASkillStepStartMode StartMode) override;
	virtual void OnTimedStepStopped() override;
	virtual void OnTimedStepElapsed() override;

	UFUNCTION()
	void HandleInputReleased(float TimeHeld);

	void ArmInputRelease();
	void StopWaitingInputRelease();
	void CommitChargeDamageMultiplier() const;
	float ResolveChargeRatio() const;

	UPROPERTY(EditDefaultsOnly, Category="Charge", meta=(ClampMin="0.0"))
	float ChargeDuration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Charge", meta=(ClampMin="1.0"))
	float FullChargeFinalDamageMultiplier = 1.f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;

	UPROPERTY(Transient)
	float ChargeStartTime = -1.f;
};
