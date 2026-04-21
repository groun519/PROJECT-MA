#include "Widget/MAStepProgressWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/MASkillAbility.h"

void UMAStepProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ClearStepProgress();
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UMAStepProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshFromOwner();
}

void UMAStepProgressWidget::SetStepProgress(const FText& InLabel, float InDuration, float InRemainingDuration)
{
	LabelText->SetText(InLabel);
	DurationProgressBar->SetPercent(InDuration > 0.f ? FMath::Clamp(InRemainingDuration / InDuration, 0.f, 1.f) : 0.f);
	SetRenderOpacity(1.f);
}

void UMAStepProgressWidget::ClearStepProgress()
{
	LabelText->SetText(FText::GetEmpty());
	DurationProgressBar->SetPercent(0.f);
	SetRenderOpacity(0.f);
}

void UMAStepProgressWidget::RefreshFromOwner()
{
	UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (!OwnerAbilitySystemComponent)
	{
		ClearStepProgress();
		return;
	}

	const TArray<FGameplayAbilitySpec>& AbilitySpecs = OwnerAbilitySystemComponent->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySpecs)
	{
		if (!AbilitySpec.IsActive()) continue;

		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			UMASkillAbility* SkillAbility = Cast<UMASkillAbility>(AbilityInstance);
			if (!SkillAbility) continue;

			FText StepLabel;
			float StepDuration = 0.f;
			float StepRemainingDuration = 0.f;
			if (SkillAbility->GetSkillProgressInfo(StepLabel, StepDuration, StepRemainingDuration))
			{
				SetStepProgress(StepLabel, StepDuration, StepRemainingDuration);
				return;
			}
		}
	}

	ClearStepProgress();
}
