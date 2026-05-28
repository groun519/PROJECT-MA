#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Step/MASkillStep.h"
#include "UObject/Object.h"
#include "MASkillStepManager.generated.h"

class UMASkillAbility;
class UMASkillModuleInstance;
struct FGameplayEventData;

UCLASS()
class P_MA_API UMASkillStepManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UMASkillAbility* SkillAbility);
	void ResetRuntimeState();
	void UpdateSteps(const TArray<TObjectPtr<UMASkillStep>>& InRuntimeSteps);

	UMASkillStep* GetRuntimeSkillStep(int32 StepIndex) const;
	UMASkillStep* GetCurrentRuntimeSkillStep() const { return GetRuntimeSkillStep(CurrentStepIndex); }
	bool SetActiveStep(int32 TargetStepIndex, EMASkillStepStartMode StartMode);
	bool TransitionToStep(int32 TargetStepIndex, EMASkillStepStartMode StartMode, float MontageBlendOutTime);
	bool TryTransitionToPreparedStep(int32 TargetStepIndex);
	void AdvanceOrEnd(int32 NextStepIndex, float MontageBlendOutTime);
	void HandleRuntimeEvent(const FGameplayEventData& EventData) const;
	void SetDesiredMontagePlayRate(float NewPlayRate);
	float GetDesiredMontagePlayRate() const { return DesiredMontagePlayRate; }
	void ApplyDesiredMontagePlayRate() const;
	bool GetSkillProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const;
	void StopActiveStep(float MontageBlendOutTime = 0.f);
	void ClearPreparedStepPreviews(int32 ExceptStepIndex = INDEX_NONE) const;
	bool IsCurrentStepPrepared() const { return CurrentStepStartMode == EMASkillStepStartMode::Prepared; }
	UMASkillModuleInstance* GetCurrentBindingScope() const;

private:
	void HandleSkillActivated();
	void HandleSkillDeactivated();

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	UPROPERTY(Transient)
	int32 CurrentStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	EMASkillStepStartMode CurrentStepStartMode = EMASkillStepStartMode::Fresh;

	UPROPERTY(Transient)
	bool bInitialized = false;

	UPROPERTY(Transient)
	float DesiredMontagePlayRate = 1.f;

	const TArray<TObjectPtr<UMASkillStep>>* RuntimeSteps = nullptr;
};
