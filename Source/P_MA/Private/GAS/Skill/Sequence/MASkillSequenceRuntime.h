#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Sequence/MASkillSequenceTypes.h"
#include "UObject/Object.h"
#include "MASkillSequenceRuntime.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;
class UMASkillAbility;
class UMASkillSequenceTask;
struct FMASkillSequenceTaskCompletionEvent;

UCLASS()
class P_MA_API UMASkillSequenceRuntime : public UObject
{
	GENERATED_BODY()

public:
	void UpdateSequence(const TArray<FMASkillSequence>& InSequences);
	void ResetSequence();
	void Start();
	void Stop();
	bool GetProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const;
	bool PrepareCurrentMontage(float PreparationTime);
	void SetDesiredPlayRate(float PlayRate);
	float GetDesiredPlayRate() const { return DesiredPlayRate; }
	const FMASkillScopes* GetCurrentTargetScopes() const;
	void NotifyTaskCompletionEvent(
		const FMASkillScopes& SourceScopes,
		const FMASkillSequenceTaskCompletionEvent& CompletionEvent,
		float ProgressRatio);

private:
	UMASkillAbility& GetOwnerAbility() const;
	const FMASkillSequence* GetCurrentSequence() const;
	void Abort();
	void ExecuteCurrentSequence();
	void ExecuteCurrentTask();
	void PlayCurrentMontage();
	void CompleteCurrentSequence();
	void Complete();
	void ClearMontageTask();
	void ClearPreparedMontage();
	void StopCurrentMontage();
	void RegisterAnimationOwner(UAnimMontage& Montage) const;
	void UnregisterAnimationOwner(UAnimMontage& Montage) const;
	void BindPreparedMontageDelegates(UAnimMontage& Montage);
	void ApplyCurrentMontagePlayRate();
	FName ResolveStartSectionName() const;
	void AdvanceSectionIndex();
	void HandleTaskFinished(UMASkillSequenceTask* FinishedTask, bool bAborted);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageAborted();

	void HandlePreparedMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void HandlePreparedMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(Transient)
	TArray<FMASkillSequence> Sequences;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillSequenceTask> ActiveTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TArray<int32> SectionIndices;

	int32 CurrentSequenceIndex = INDEX_NONE;
	int32 CurrentTaskIndex = INDEX_NONE;
	float DesiredPlayRate = 1.f;
	bool bRunning = false;
	bool bMontagePrepared = false;
	bool bPreparedMontageActive = false;
};
