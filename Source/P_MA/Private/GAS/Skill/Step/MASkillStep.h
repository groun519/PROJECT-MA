#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "MASkillStep.generated.h"

class UAnimMontage;
class UMASkillAbility;
class UMASkillStepManager;
class UAnimInstance;
class UAbilityTask_PlayMontageAndWait;
class UMASkillModuleInstance;
struct FGameplayEventData;

UENUM()
enum class EMASkillStepStartMode : uint8
{
	Fresh,
	Prepared
};

USTRUCT()
struct FMASkillStepSequenceState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 CurrentIndex = 0;

	UPROPERTY(Transient)
	int32 AdvanceCount = 0;

	UPROPERTY(Transient)
	int32 MaxIndex = 0;
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillStep : public UObject
{
	GENERATED_BODY()

public:
	void ConfigureAssembledStep(
		int32 InStepIndex,
		int32 InNextStepIndex,
		int32 InNextMontageStepIndex,
		int32 InInitialSequenceIndex = 0,
		int32 InSequenceAdvanceCount = 0);
	void BindRuntimeSkillAbility(UMASkillAbility* SkillAbility) { OwnerSkillAbility = SkillAbility; }
	void SetBindingScope(UMASkillModuleInstance* InBindingScope) { BindingScope = InBindingScope; }
	UMASkillModuleInstance* GetBindingScope() const { return BindingScope; }

	virtual void StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode StartMode);
	void EnterStep(EMASkillStepStartMode StartMode)
	{
		StartStep(GetOwnerSkillAbility(), StartMode);
		if (StartMode == EMASkillStepStartMode::Prepared)
		{
			BroadcastMontageStartedEvent();
			return;
		}

		StartCurrentStepMontage();
	}

	void StopActiveStep(float MontageBlendOutTime = 0.f)
	{
		StopStep();
		ClearCurrentMontageTask();
		StopCurrentStepMontage(MontageBlendOutTime);
		ClearPreparedStepPreview();
	}

	void ApplyDesiredMontagePlayRate(float DesiredMontagePlayRate) const
	{
		UAnimInstance* AnimInstance = nullptr;
		UAnimMontage* CurrentStepMontage = nullptr;
		if (!TryResolveStepMontageContext(AnimInstance, CurrentStepMontage) || !AnimInstance->Montage_IsPlaying(CurrentStepMontage))
			return;

		AnimInstance->Montage_SetPlayRate(CurrentStepMontage, DesiredMontagePlayRate);
	}

	virtual void StopStep() {}
	virtual void HandleStepMontageCancelled();
	virtual void HandleStepMontageCompleted();
	virtual void HandleStepMontageInterrupted() { HandleStepMontageCancelled(); }

	virtual UAnimMontage* ResolveStepMontage() const { return StepMontage; }
	virtual FName ResolveStepStartSectionName() const;
	bool UsesSequenceSections() const { return !SequenceSectionNameBase.IsNone(); }
	FString GetSequenceSectionKey() const;
	bool PrepareNextStepPreview(float PreviewBlendInTime);
	bool HasPreparedStepPreview() const { return PreparedStepPreviewMontage != nullptr; }
	bool PromotePreparedStepPreviewToActive();
	void ClearPreparedStepPreview(float BlendOutTime = 0.f);
	virtual bool ShouldAutoAdvanceOnMontageCompleted() const { return true; }
	virtual bool GetStepProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const { return false; }
	virtual void HandleRuntimeEvent(const FGameplayEventData& EventData) {}

protected:
	UMASkillAbility* GetOwnerSkillAbility() const { return OwnerSkillAbility; }
	UMASkillStepManager* GetOwnerStepManager() const;
	void RequestAdvanceOrEnd(float MontageBlendOutTime = 0.f);
	UAnimInstance* ResolveOwnerAnimInstance() const;

	UPROPERTY(EditDefaultsOnly, Category="Step")
	TObjectPtr<UAnimMontage> StepMontage;

	// Steps with the same montage and sequence section base are one visual sequence group.
	// Keep each montage + section base pair unique inside a module unless shared sequencing is intended.
	UPROPERTY(EditDefaultsOnly, Category="Step", meta=(DisplayName="SequenceSectionNameBase"))
	FName SequenceSectionNameBase = NAME_None;

	UPROPERTY(EditDefaultsOnly, Category="Step", meta=(ClampMin="0", DisplayName="MaxSectionIndex"))
	int32 MaxSequenceSectionCount = 0;

	// TODO: If step variants grow and repeat the same task-owner usage patterns, replace this with a narrower step task-owner interface.
	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerSkillAbility;

	UPROPERTY(Transient)
	int32 StepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 NextStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 NextMontageStepIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FMASkillStepSequenceState SequenceState;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> PreparedStepPreviewMontage;

	UPROPERTY(Transient)
	bool bPreparedStepPreviewActivated = false;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModuleInstance> BindingScope = nullptr;

	int32 ResolveCurrentSequenceIndex() const;
	FName MakeSequenceSectionName(int32 SectionIndex) const;
	void AdvanceSequence();
	UMASkillStep* ResolveNextMontageRuntimeStep() const;
	bool PrepareStepPreview(float PreviewBlendInTime);
	bool TryResolveStepMontageContext(UAnimInstance*& OutAnimInstance, UAnimMontage*& OutStepMontage) const;
	void BroadcastMontageStartedEvent() const;
	void StartCurrentStepMontage();
	void ClearCurrentMontageTask();
	void StopCurrentStepMontage(float MontageBlendOutTime = 0.f);
	UAnimMontage* ReleasePreparedStepPreview(bool bKeepAnimationOwnerRegistration);
	void FinalizePreparedStepPreview(UAnimMontage* Montage, bool bInterrupted);
	void BindPreparedStepPreviewDelegates(UAnimMontage* Montage);
	void ClearPreparedStepPreviewDelegates(UAnimMontage* Montage);

	UFUNCTION()
	void HandleCurrentStepMontageCompletedTask();

	UFUNCTION()
	void HandleCurrentStepMontageFailedTask();

	UFUNCTION()
	void HandlePreparedStepPreviewBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandlePreparedStepPreviewEnded(UAnimMontage* Montage, bool bInterrupted);
};
