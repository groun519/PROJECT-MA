#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "TimerManager.h"
#include "MASkillFlowPart_Charge.generated.h"

class UAbilityTask_WaitInputRelease;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories="Flow")
class P_MA_API UMASkillFlowPart_Charge : public UMASkillFlowPart
{
	GENERATED_BODY()

public:
	virtual void StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode) override;
	virtual void StopFlow() override;

private:
	UFUNCTION()
	void HandleInputReleased(float TimeHeld);

	UFUNCTION()
	void HandleChargeDurationElapsed();

	void ArmInputRelease();
	void StopWaitingInputRelease();
	void StartChargeDurationTimer();
	void StopChargeDurationTimer();
	void CommitChargePayload() const;
	void ActivateNextFlow();
	float ResolveChargeRatio() const;

	UPROPERTY(EditDefaultsOnly, Category="Charge", meta=(ClampMin="0.0"))
	float ChargeDuration = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;

	UPROPERTY(Transient)
	float ChargeStartTime = -1.f;

	FTimerHandle ChargeDurationTimerHandle;
};
