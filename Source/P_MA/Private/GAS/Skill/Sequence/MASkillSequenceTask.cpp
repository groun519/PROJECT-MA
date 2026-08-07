#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Animation/MAAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Sequence/MASkillSequenceRuntime.h"

void UMASkillSequenceTask::Configure(const FMASkillSequenceTaskConfig& InConfig)
{
	Config = InConfig;
	Config.TimeLimitSeconds = FMath::Max(Config.TimeLimitSeconds, 0.f);
}

void UMASkillSequenceTask::Start(
	UMASkillSequenceRuntime& InRuntime,
	UMASkillAbility& InAbility,
	const FMASkillScopes& InTargetScopes,
	FMASkillSequenceTaskFinishedSignature InFinishedDelegate)
{
	SequenceRuntime = &InRuntime;
	SkillAbility = &InAbility;
	TargetScopes = InTargetScopes;
	FinishedDelegate = MoveTemp(InFinishedDelegate);
	bFinished = false;
	StartConfiguredBehaviors();
}

void UMASkillSequenceTask::Abort()
{
	Finish(true);
}

bool UMASkillSequenceTask::GetProgressInfo(
	FText& OutLabel,
	float& OutDuration,
	float& OutRemainingDuration) const
{
	if (!Config.bShowProgress || Config.TimeLimitSeconds <= 0.f) return false;

	OutLabel = Config.ProgressLabel;
	OutDuration = Config.TimeLimitSeconds;
	OutRemainingDuration = GetRemainingTime();
	return true;
}

void UMASkillSequenceTask::ApplyPlayRate(float PlayRate)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		ASC && Config.CustomMontage && ASC->GetCurrentMontage() == Config.CustomMontage)
	{
		ASC->CurrentMontageSetPlayRate(PlayRate);
	}
}

void UMASkillSequenceTask::StartConfiguredBehaviors()
{
	ProgressStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	ProgressEndTimeSeconds = ProgressStartTimeSeconds + Config.TimeLimitSeconds;

	ApplyInputBlock();
	if (IsFinished()) return;

	if (!StartMontageBehavior())
	{
		Abort();
		return;
	}
	if (IsFinished()) return;

	if (!StartInputReleaseWait())
	{
		Abort();
		return;
	}
	if (IsFinished()) return;

	if (Config.TimeLimitSeconds > 0.f)
	{
		StartTimer();
	}
	else if (!Config.bWaitInputRelease
		&& Config.MontageMode != EMASkillSequenceTaskMontageMode::CustomMontage)
	{
		Complete();
	}
}

void UMASkillSequenceTask::StopConfiguredBehaviors()
{
	ClearTimer();
	ClearInputReleaseWait();
	ClearCustomMontageTask();
	ClearInputBlock();
	ProgressStartTimeSeconds = 0.f;
	ProgressEndTimeSeconds = 0.f;
}

void UMASkillSequenceTask::Complete()
{
	Finish(false);
}

void UMASkillSequenceTask::Finish(bool bAborted)
{
	if (bFinished) return;

	const float CompletionProgressRatio = GetProgressRatio();
	bFinished = true;
	StopConfiguredBehaviors();
	if (!bAborted)
	{
		NotifyCompletionEvent(CompletionProgressRatio);
	}
	FMASkillSequenceTaskFinishedSignature Callback = MoveTemp(FinishedDelegate);
	FinishedDelegate.Unbind();
	Callback.ExecuteIfBound(this, bAborted);
}

void UMASkillSequenceTask::StartTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Abort();
		return;
	}

	World->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&UMASkillSequenceTask::HandleTimerElapsed,
		Config.TimeLimitSeconds,
		false);
}

void UMASkillSequenceTask::ClearTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
}

void UMASkillSequenceTask::ApplyInputBlock()
{
	if (!Config.bBlockInput) return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
		bAppliedInputBlockTag = true;
	}
}

void UMASkillSequenceTask::ClearInputBlock()
{
	if (!bAppliedInputBlockTag) return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
	}
	bAppliedInputBlockTag = false;
}

bool UMASkillSequenceTask::StartMontageBehavior()
{
	switch (Config.MontageMode)
	{
	case EMASkillSequenceTaskMontageMode::PrepareCurrentMontage:
		return GetSequenceRuntime()
			&& GetSequenceRuntime()->PrepareCurrentMontage(Config.TimeLimitSeconds);

	case EMASkillSequenceTaskMontageMode::CustomMontage:
		break;

	default:
		return true;
	}

	UMASkillAbility* OwnerAbility = GetSkillAbility();
	if (!OwnerAbility || !Config.CustomMontage) return false;

	CustomMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		OwnerAbility,
		NAME_None,
		Config.CustomMontage,
		GetSequenceRuntime() ? GetSequenceRuntime()->GetCurrentPlayRate() : 1.f);
	if (!CustomMontageTask) return false;

	CustomMontageTask->OnCancelled.AddDynamic(this, &UMASkillSequenceTask::HandleCustomMontageFailed);
	CustomMontageTask->OnInterrupted.AddDynamic(this, &UMASkillSequenceTask::HandleCustomMontageFailed);
	if (Config.TimeLimitSeconds <= 0.f)
	{
		CustomMontageTask->OnBlendOut.AddDynamic(this, &UMASkillSequenceTask::HandleCustomMontageCompleted);
		CustomMontageTask->OnCompleted.AddDynamic(this, &UMASkillSequenceTask::HandleCustomMontageCompleted);
	}

	if (USkeletalMeshComponent* Mesh = OwnerAbility->GetOwningComponentFromActorInfo())
	{
		if (UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(Mesh->GetAnimInstance()))
		{
			AnimInstance->RegisterAnimationOwner(Config.CustomMontage, OwnerAbility);
		}
	}

	CustomMontageTask->ReadyForActivation();
	return true;
}

void UMASkillSequenceTask::ClearCustomMontageTask()
{
	if (CustomMontageTask)
	{
		CustomMontageTask->OnBlendOut.Clear();
		CustomMontageTask->OnCompleted.Clear();
		CustomMontageTask->OnCancelled.Clear();
		CustomMontageTask->OnInterrupted.Clear();
		CustomMontageTask->EndTask();
		CustomMontageTask = nullptr;
	}

	if (Config.MontageMode != EMASkillSequenceTaskMontageMode::CustomMontage || !Config.CustomMontage) return;

	UMASkillAbility* OwnerAbility = GetSkillAbility();
	USkeletalMeshComponent* Mesh = OwnerAbility ? OwnerAbility->GetOwningComponentFromActorInfo() : nullptr;
	if (UMAAnimInstance* AnimInstance = Mesh ? Cast<UMAAnimInstance>(Mesh->GetAnimInstance()) : nullptr)
	{
		AnimInstance->UnregisterAnimationOwner(Config.CustomMontage, OwnerAbility);
	}

	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		ASC->StopMontageIfCurrent(*Config.CustomMontage, 0.f);
	}
}

bool UMASkillSequenceTask::StartInputReleaseWait()
{
	if (!Config.bWaitInputRelease) return true;

	UMASkillAbility* OwnerAbility = GetSkillAbility();
	if (!OwnerAbility) return false;

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwnerAbility, false);
	if (!InputReleaseTask) return false;

	InputReleaseTask->OnRelease.AddDynamic(this, &UMASkillSequenceTask::HandleInputReleased);
	InputReleaseTask->ReadyForActivation();
	return true;
}

void UMASkillSequenceTask::ClearInputReleaseWait()
{
	if (!InputReleaseTask) return;

	InputReleaseTask->OnRelease.Clear();
	InputReleaseTask->EndTask();
	InputReleaseTask = nullptr;
}

void UMASkillSequenceTask::NotifyCompletionEvent(float ProgressRatio) const
{
	if (!Config.CompletionEvent.IsValid()) return;

	if (UMASkillSequenceRuntime* Runtime = GetSequenceRuntime())
	{
		Runtime->NotifyTaskCompletionEvent(TargetScopes, Config.CompletionEvent, ProgressRatio);
	}
}

float UMASkillSequenceTask::GetElapsedTime() const
{
	const UWorld* World = GetWorld();
	return World
		? FMath::Max(World->GetTimeSeconds() - ProgressStartTimeSeconds, 0.f)
		: 0.f;
}

float UMASkillSequenceTask::GetRemainingTime() const
{
	const UWorld* World = GetWorld();
	return World && ProgressEndTimeSeconds > 0.f
		? FMath::Max(ProgressEndTimeSeconds - World->GetTimeSeconds(), 0.f)
		: 0.f;
}

float UMASkillSequenceTask::GetProgressRatio() const
{
	return Config.TimeLimitSeconds > 0.f
		? FMath::Clamp(GetElapsedTime() / Config.TimeLimitSeconds, 0.f, 1.f)
		: 1.f;
}

void UMASkillSequenceTask::HandleTimerElapsed()
{
	if (Config.bCompleteOnTimeLimit)
	{
		Complete();
	}
}

void UMASkillSequenceTask::HandleCustomMontageCompleted()
{
	Complete();
}

void UMASkillSequenceTask::HandleCustomMontageFailed()
{
	Abort();
}

void UMASkillSequenceTask::HandleInputReleased(float /*TimeHeld*/)
{
	Complete();
}

UAbilitySystemComponent* UMASkillSequenceTask::GetAbilitySystemComponent() const
{
	UMASkillAbility* OwnerAbility = GetSkillAbility();
	return OwnerAbility ? OwnerAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
}
