#include "GAS/Skill/Step/MASkillStep_Cast.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"

UMASkillStep_Cast::UMASkillStep_Cast()
{
	StepProgressSettings.bShowProgress = true;
	StepProgressSettings.Label = FText::FromString(TEXT("Cast"));
}

void UMASkillStep_Cast::Configure(
	float InCastDuration,
	EMASkillCastMontageMode InMontageMode,
	UAnimMontage* InCustomMontage)
{
	CastDuration = FMath::Max(InCastDuration, 0.f);
	MontageMode = InMontageMode;
	StepMontage = MontageMode == EMASkillCastMontageMode::CustomMontage
		? InCustomMontage
		: nullptr;
}

UAnimMontage* UMASkillStep_Cast::ResolveStepMontage() const
{
	return MontageMode == EMASkillCastMontageMode::CustomMontage
		? Super::ResolveStepMontage()
		: nullptr;
}

void UMASkillStep_Cast::OnTimedStepStarted(UMASkillAbility*, EMASkillStepStartMode StartMode)
{
	ApplyInputBlockTag();
	if (MontageMode == EMASkillCastMontageMode::BlendInNextMontage
		&& StartMode == EMASkillStepStartMode::Fresh)
	{
		PrepareNextStepPreview(CastDuration);
	}
}

void UMASkillStep_Cast::OnTimedStepStopped()
{
	RemoveInputBlockTag();
}

void UMASkillStep_Cast::ApplyInputBlockTag()
{
	if (bAppliedInputBlockTag) return;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAbilitySystemComponent* ASC = SkillAbility ? SkillAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (!ASC) return;

	ASC->AddLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
	bAppliedInputBlockTag = true;
}

void UMASkillStep_Cast::RemoveInputBlockTag()
{
	if (!bAppliedInputBlockTag) return;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAbilitySystemComponent* ASC = SkillAbility ? SkillAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
	bAppliedInputBlockTag = false;
}
