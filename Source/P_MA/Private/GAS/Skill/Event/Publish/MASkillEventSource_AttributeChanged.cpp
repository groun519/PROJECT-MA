#include "GAS/Skill/Event/Publish/MASkillEventSource_AttributeChanged.h"

#include "AbilitySystemComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GameplayTagsManager.h"

static FGameplayTag BuildAttributeChangedEventTag(const FGameplayAttribute& Attribute)
{
	if (!Attribute.IsValid()) return FGameplayTag();

	const FString TagString = FString::Printf(TEXT("Event.Attribute.%s.Changed"), *Attribute.GetName());
	return UGameplayTagsManager::Get().RequestGameplayTag(FName(*TagString), false);
}

UMASkillEventSource_AttributeChanged::UMASkillEventSource_AttributeChanged()
{
	RefreshEmittedTag();
}

void UMASkillEventSource_AttributeChanged::StartSource(UMASkillAbility* SkillAbility)
{
	Super::StartSource(SkillAbility);
	RefreshEmittedTag();

	if (!ChangedAttribute.IsValid()) return;

	UAbilitySystemComponent* AbilitySystemComponent = SkillAbility ? SkillAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (!AbilitySystemComponent) return;

	AttributeChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(ChangedAttribute)
		.AddUObject(this, &UMASkillEventSource_AttributeChanged::HandleAttributeChanged);
}

void UMASkillEventSource_AttributeChanged::StopSource()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetOwnerSkillAbility() ? GetOwnerSkillAbility()->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (AbilitySystemComponent && AttributeChangedHandle.IsValid() && ChangedAttribute.IsValid())
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(ChangedAttribute)
			.Remove(AttributeChangedHandle);
	}

	AttributeChangedHandle.Reset();
	Super::StopSource();
}

void UMASkillEventSource_AttributeChanged::PostLoad()
{
	Super::PostLoad();
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
	(void)ChangeData;
	EmitEvent();
}

void UMASkillEventSource_AttributeChanged::RefreshEmittedTag()
{
	EmittedTag = BuildAttributeChangedEventTag(ChangedAttribute);
}
