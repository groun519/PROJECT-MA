#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Input/MASkillFlowPart_Timed.h"
#include "MASkillFlowPart_Charge.generated.h"

class UAbilityTask_WaitInputRelease;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories="Flow")
class P_MA_API UMASkillFlowPart_Charge : public UMASkillFlowPart_Timed
{
	GENERATED_BODY()

public:
	UMASkillFlowPart_Charge();

private:
	virtual float GetFlowDuration() const override { return ChargeDuration; }
	virtual void OnTimedFlowStarted(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode) override;
	virtual void OnTimedFlowStopped() override;
	virtual void OnTimedFlowElapsed() override;

	UFUNCTION()
	void HandleInputReleased(float TimeHeld);

	void ArmInputRelease();
	void StopWaitingInputRelease();
	void CommitChargePayload() const;
	float ResolveChargeRatio() const;

	UPROPERTY(EditDefaultsOnly, Category="Charge", meta=(ClampMin="0.0"))
	float ChargeDuration = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;

	UPROPERTY(Transient)
	float ChargeStartTime = -1.f;
};
