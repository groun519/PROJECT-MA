#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "TimerManager.h"
#include "MASkillFlowPart_Delay.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, HideCategories="Flow")
class P_MA_API UMASkillFlowPart_Delay : public UMASkillFlowPart
{
	GENERATED_BODY()

public:
	virtual UAnimMontage* ResolveFlowMontage() const override { return nullptr; }
	virtual void StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode) override;
	virtual void StopFlow() override;
	virtual bool ShouldAutoAdvanceOnMontageCompleted() const override;

private:
	UFUNCTION()
	void HandleDelayElapsed();

	void StartDelayTimer();
	void StopDelayTimer();
	void ActivateNextFlow();

	UPROPERTY(EditDefaultsOnly, Category="Delay", meta=(ClampMin="0.0"))
	float DelayDuration = 0.f;

	FTimerHandle DelayTimerHandle;
};
