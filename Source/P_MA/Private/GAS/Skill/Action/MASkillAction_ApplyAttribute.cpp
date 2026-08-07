#include "GAS/Skill/Action/MASkillAction_ApplyAttribute.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"
#include "GameFramework/Actor.h"

namespace
{
	const FName LostHealthPayloadTagName(TEXT("Data.Health.Lost"));
}

void UMASkillAction_ApplyAttribute::PostLoad()
{
	Super::PostLoad();

	for (int32 Index = AttributeCoefficients.Num() - 1; Index >= 0; --Index)
	{
		const FMAAttributeCoefficient& Coefficient = AttributeCoefficients[Index];
		if (Coefficient.Source != EMACoefficientSource::Payload
			|| Coefficient.PayloadTag.GetTagName() != LostHealthPayloadTagName)
		{
			continue;
		}

		EventMagnitudeCoefficient += Coefficient.Coefficient;
		AttributeCoefficients.RemoveAt(Index);
	}
}

void UMASkillAction_ApplyAttribute::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!Owner.HasAuthority() || !Attribute.IsValid()) return;

	UAbilitySystemComponent* SourceASC = Ability->GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	const FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	UAbilitySystemComponent* TargetASC = SourceASC;
	if (Target == EMASkillAttributeApplyTarget::TargetPayload)
	{
		if (!TargetPayloadTag.IsValid()) return;

		UObject* TargetObject = nullptr;
		if (!Payloads.Reader.TryGetObject(TargetPayloadTag, TargetObject)) return;

		TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Cast<AActor>(TargetObject));
		if (!TargetASC) return;
	}

	float FinalValue = BaseValue + Event.GetMagnitude() * EventMagnitudeCoefficient;
	for (const FMAAttributeCoefficient& Coefficient : AttributeCoefficients)
	{
		FinalValue += Coefficient.ResolveValue(*SourceASC, *TargetASC, Payloads);
	}
	if (FMath::IsNearlyZero(FinalValue)) return;

	TargetASC->ApplyModToAttribute(Attribute, Operation, FinalValue);
}
