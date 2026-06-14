#include "GAS/Skill/Event/Source/MASkillEventSource_AttributeChanged.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GameplayTagsManager.h"

static const TCHAR* GetAttributeChangeTypeName(EMAAttributeChangeType ChangeType)
{
	switch (ChangeType)
	{
	case EMAAttributeChangeType::Increased: return TEXT("Increased");
	case EMAAttributeChangeType::Decreased: return TEXT("Decreased");
	default: return TEXT("Changed");
	}
}

static FGameplayTag BuildAttributeChangedEventTag(const FGameplayAttribute& Attribute, EMAAttributeChangeType ChangeType)
{
	if (!Attribute.IsValid()) return FGameplayTag();

	const FString TagString = FString::Printf(
		TEXT("Event.Attribute.%s.%s"),
		*Attribute.GetName(),
		GetAttributeChangeTypeName(ChangeType));
	return UGameplayTagsManager::Get().RequestGameplayTag(FName(*TagString), false);
}

UMASkillEventSource_AttributeChanged::UMASkillEventSource_AttributeChanged()
{
	RefreshEmittedTag();
}

void UMASkillEventSource_AttributeChanged::StartSource()
{
	RefreshEmittedTag();

	if (!ChangedAttribute.IsValid()) return;

	const UMASkillManagerComponent* SkillManager = GetSkillManager();
	UAbilitySystemComponent* AbilitySystemComponent = SkillManager
		? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SkillManager->GetOwner())
		: nullptr;
	if (!AbilitySystemComponent) return;

	AttributeChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(ChangedAttribute)
		.AddUObject(this, &UMASkillEventSource_AttributeChanged::HandleAttributeChanged);
}

void UMASkillEventSource_AttributeChanged::StopSource()
{
	const UMASkillManagerComponent* SkillManager = GetSkillManager();
	UAbilitySystemComponent* AbilitySystemComponent = SkillManager
		? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SkillManager->GetOwner())
		: nullptr;
	if (AbilitySystemComponent && AttributeChangedHandle.IsValid() && ChangedAttribute.IsValid())
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(ChangedAttribute)
			.Remove(AttributeChangedHandle);
	}

	AttributeChangedHandle.Reset();
}

void UMASkillEventSource_AttributeChanged::PostLoad()
{
	Super::PostLoad();

	static const FGameplayTag HealthDecreasedTag =
		FGameplayTag::RequestGameplayTag(TEXT("Event.Attribute.Health.Decreased"), false);
	if (!ChangedAttribute.IsValid() && EmittedTag == HealthDecreasedTag)
	{
		ChangedAttribute = UMAAttributeSet::GetHealthAttribute();
		ChangeType = EMAAttributeChangeType::Decreased;
	}

	RefreshEmittedTag();
}

#if WITH_EDITOR
void UMASkillEventSource_AttributeChanged::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshEmittedTag();
}
#endif

void UMASkillEventSource_AttributeChanged::HandleAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	float Magnitude = ChangeData.NewValue - ChangeData.OldValue;
	if (ChangeType == EMAAttributeChangeType::Increased && Magnitude <= 0.f) return;
	if (ChangeType == EMAAttributeChangeType::Decreased)
	{
		if (Magnitude >= 0.f) return;
		Magnitude *= -1.f;
	}

	FMASkillEvent Event(EmittedTag);
	Event.SetMagnitude(Magnitude);
	UMASkillEventRoutingStatics::TryNotifyGlobalEvent(GetSkillManager(), MoveTemp(Event));
}

void UMASkillEventSource_AttributeChanged::RefreshEmittedTag()
{
	EmittedTag = BuildAttributeChangedEventTag(ChangedAttribute, ChangeType);
}
