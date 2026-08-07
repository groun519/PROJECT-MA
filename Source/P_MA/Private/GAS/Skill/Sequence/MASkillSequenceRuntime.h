#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Sequence/MASkillSequenceTypes.h"
#include "UObject/Object.h"
#include "MASkillSequenceRuntime.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;
class UMASkillAbility;
class UMASkillSequenceTask;
struct FOnAttributeChangeData;
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
	const FMASkillScopes* GetCurrentTargetScopes() const;
	void NotifyTaskCompletionEvent(
		const FMASkillScopes& SourceScopes,
		const FMASkillSequenceTaskCompletionEvent& CompletionEvent,
		float ProgressRatio);

	/** Play Rate **/
	void RefreshPlayRate();
	float GetCurrentPlayRate() const { return CurrentPlayRate; }

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
	FName ResolveStartSectionName() const;
	void AdvanceSectionIndex();
	void HandleTaskFinished(UMASkillSequenceTask* FinishedTask, bool bAborted);

	/** Play Rate **/
	void ApplyCurrentMontagePlayRate();
	float GetCurrentSectionPlayLength() const;
	void BindAttackSpeedChanged();
	void UnbindAttackSpeedChanged();
	void HandleAttackSpeedChanged(const FOnAttributeChangeData& ChangeData);

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
	FName CurrentSectionName = NAME_None;
	bool bRunning = false;
	bool bMontagePrepared = false;
	bool bPreparedMontageActive = false;

	/** Play Rate **/
	float CurrentPlayRate = 1.f;
	FDelegateHandle AttackSpeedChangedHandle;
};
