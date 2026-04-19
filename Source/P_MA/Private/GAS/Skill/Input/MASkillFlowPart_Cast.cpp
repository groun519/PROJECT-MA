#include "GAS/Skill/Input/MASkillFlowPart_Cast.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"

UMASkillFlowPart_Cast::UMASkillFlowPart_Cast()
{
	FlowProgressSettings.bShowProgress = true;
	FlowProgressSettings.Label = FText::FromString(TEXT("Cast"));
}

void UMASkillFlowPart_Cast::OnTimedFlowStarted(UMASkillAbility*, EMASkillFlowStartMode)
{
	ApplyInputBlockTag();
}

void UMASkillFlowPart_Cast::OnTimedFlowStopped()
{
	RemoveInputBlockTag();
}

void UMASkillFlowPart_Cast::ApplyInputBlockTag()
{
	if (bAppliedInputBlockTag) return;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAbilitySystemComponent* ASC = SkillAbility ? SkillAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (!ASC) return;

	ASC->AddLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
	bAppliedInputBlockTag = true;
}

void UMASkillFlowPart_Cast::RemoveInputBlockTag()
{
	if (!bAppliedInputBlockTag) return;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAbilitySystemComponent* ASC = SkillAbility ? SkillAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
	bAppliedInputBlockTag = false;
}
