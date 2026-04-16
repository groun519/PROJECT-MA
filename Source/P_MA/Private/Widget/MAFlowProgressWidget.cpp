#include "Widget/MAFlowProgressWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Input/MASkillFlowPart_Timed.h"

void UMAFlowProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ClearFlowProgress();
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UMAFlowProgressWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshFromOwner();
}

void UMAFlowProgressWidget::SetFlowProgress(const FText& InLabel, float InDuration, float InRemainingDuration)
{
	LabelText->SetText(InLabel);
	DurationProgressBar->SetPercent(InDuration > 0.f ? FMath::Clamp(InRemainingDuration / InDuration, 0.f, 1.f) : 0.f);
	SetRenderOpacity(1.f);
}

void UMAFlowProgressWidget::ClearFlowProgress()
{
	LabelText->SetText(FText::GetEmpty());
	DurationProgressBar->SetPercent(0.f);
	SetRenderOpacity(0.f);
}

void UMAFlowProgressWidget::RefreshFromOwner()
{
	UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (!OwnerAbilitySystemComponent)
	{
		ClearFlowProgress();
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

			const UMASkillFlowPart_Timed* TimedFlowPart = Cast<UMASkillFlowPart_Timed>(SkillAbility->GetCurrentRuntimeFlowPart());
			if (!TimedFlowPart) continue;

			FText FlowLabel;
			float FlowDuration = 0.f;
			float FlowRemainingDuration = 0.f;
			if (TimedFlowPart->GetFlowProgressInfo(FlowLabel, FlowDuration, FlowRemainingDuration))
			{
				SetFlowProgress(FlowLabel, FlowDuration, FlowRemainingDuration);
				return;
			}
		}
	}

	ClearFlowProgress();
}
