#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "GAS/Skill/Sequence/MASkillSequenceTypes.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "MASkillSequenceTask.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitInputRelease;
class UAnimMontage;
class UMASkillAbility;
class UMASkillSequenceRuntime;
class UMASkillSequenceTask;
class UAbilitySystemComponent;

UENUM()
enum class EMASkillSequenceTaskMontageMode : uint8
{
	None,
	CustomMontage,
	PrepareCurrentMontage
};

USTRUCT()
struct FMASkillSequenceTaskCompletionEvent
{
	GENERATED_BODY()

	bool IsValid() const { return EventTag.IsValid(); }

	UPROPERTY()
	FGameplayTag EventTag;

	UPROPERTY()
	FGameplayTag ProgressRatioPayloadTag;
};

USTRUCT()
struct FMASkillSequenceTaskConfig
{
	GENERATED_BODY()

	UPROPERTY()
	float TimeLimitSeconds = 0.f;

	UPROPERTY()
	bool bCompleteOnTimeLimit = false;

	UPROPERTY()
	bool bShowProgress = false;

	UPROPERTY()
	FText ProgressLabel;

	UPROPERTY()
	bool bBlockInput = false;

	UPROPERTY()
	EMASkillSequenceTaskMontageMode MontageMode = EMASkillSequenceTaskMontageMode::None;

	UPROPERTY()
	TObjectPtr<UAnimMontage> CustomMontage = nullptr;

	UPROPERTY()
	bool bWaitInputRelease = false;

	UPROPERTY()
	FMASkillSequenceTaskCompletionEvent CompletionEvent;
};

DECLARE_DELEGATE_TwoParams(
	FMASkillSequenceTaskFinishedSignature,
	UMASkillSequenceTask*,
	bool);

// SequenceTask is a small runtime runner for common sequence wait/prepare behaviors.
// It is not a gameplay action extension point. If a new modifier requires new task
// fields instead of composing these options, that feature has outgrown this runner.
UCLASS()
class P_MA_API UMASkillSequenceTask : public UObject
{
	GENERATED_BODY()

public:
	void Configure(const FMASkillSequenceTaskConfig& InConfig);
	void Start(
		UMASkillSequenceRuntime& InRuntime,
		UMASkillAbility& InAbility,
		const FMASkillScopes& InTargetScopes,
		FMASkillSequenceTaskFinishedSignature InFinishedDelegate);
	void Abort();
	bool GetProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const;
	void ApplyPlayRate(float PlayRate);

private:
	void StartConfiguredBehaviors();
	void StopConfiguredBehaviors();
	void Complete();
	void Finish(bool bAborted);
	bool IsFinished() const { return bFinished; }

	void StartTimer();
	void ClearTimer();
	void ApplyInputBlock();
	void ClearInputBlock();
	bool StartMontageBehavior();
	void ClearCustomMontageTask();
	bool StartInputReleaseWait();
	void ClearInputReleaseWait();
	void NotifyCompletionEvent(float ProgressRatio) const;
	float GetElapsedTime() const;
	float GetRemainingTime() const;
	float GetProgressRatio() const;

	UFUNCTION()
	void HandleTimerElapsed();

	UFUNCTION()
	void HandleCustomMontageCompleted();

	UFUNCTION()
	void HandleCustomMontageFailed();

	UFUNCTION()
	void HandleInputReleased(float TimeHeld);

	UMASkillSequenceRuntime* GetSequenceRuntime() const { return SequenceRuntime.Get(); }
	UMASkillAbility* GetSkillAbility() const { return SkillAbility.Get(); }
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UPROPERTY()
	FMASkillSequenceTaskConfig Config;

	TWeakObjectPtr<UMASkillSequenceRuntime> SequenceRuntime;
	TWeakObjectPtr<UMASkillAbility> SkillAbility;

	UPROPERTY(Transient)
	FMASkillScopes TargetScopes;

	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CustomMontageTask;

	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;

	FMASkillSequenceTaskFinishedSignature FinishedDelegate;
	FTimerHandle TimerHandle;
	float ProgressStartTimeSeconds = 0.f;
	float ProgressEndTimeSeconds = 0.f;
	bool bAppliedInputBlockTag = false;
	bool bFinished = false;
};
