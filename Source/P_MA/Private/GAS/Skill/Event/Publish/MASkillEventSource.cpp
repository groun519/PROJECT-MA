#include "GAS/Skill/Event/Publish/MASkillEventSource.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadWriter.h"

void UMASkillEventSource::InitializeRuntime(UMASkillAbility* SkillAbility, UMASkillModuleInstance* InEventScope)
{
	if (!SkillAbility || !InEventScope) return;
	if (OwnerSkillAbility == SkillAbility && EventScope == InEventScope) return;
	if (OwnerSkillAbility) DeinitializeRuntime();

	OwnerSkillAbility = SkillAbility;
	EventScope = InEventScope;
	ScopedEventDelegateHandle = EventScope->OnScopedEvent().AddUObject(this, &UMASkillEventSource::HandleScopedEvent);
	OwnerSkillAbility->OnSkillActivated().AddUObject(this, &UMASkillEventSource::HandleSkillActivated);
	OwnerSkillAbility->OnSkillDeactivated().AddUObject(this, &UMASkillEventSource::HandleSkillDeactivated);
}

void UMASkillEventSource::DeinitializeRuntime()
{
	if (!OwnerSkillAbility) return;

	StopSource();
	if (EventScope && ScopedEventDelegateHandle.IsValid())
	{
		EventScope->OnScopedEvent().Remove(ScopedEventDelegateHandle);
	}
	ScopedEventDelegateHandle.Reset();
	OwnerSkillAbility->OnSkillActivated().RemoveAll(this);
	OwnerSkillAbility->OnSkillDeactivated().RemoveAll(this);
	EventScope = nullptr;
	OwnerSkillAbility = nullptr;
}

void UMASkillEventSource::EmitEvent() const
{
	FGameplayEventData EventData;
	EmitEvent(EventData);
}

void UMASkillEventSource::EmitEvent(const FGameplayEventData& EventData) const
{
	if (!OwnerSkillAbility || !EmittedTag.IsValid()) return;
	if (!EventScope) return;

	EmitEvent(*OwnerSkillAbility, *EventScope, EventData);
}

void UMASkillEventSource::EmitEvent(
	UMASkillAbility& SkillAbility,
	UMASkillModuleInstance& InEventScope,
	const FGameplayEventData& EventData) const
{
	if (!EmittedTag.IsValid()) return;

	FGameplayEventData OutgoingEventData = EventData;
	OutgoingEventData.EventTag = EmittedTag;

	for (UMASkillPayloadWriter* PayloadWriter : PreEmitPayloadWriters)
	{
		if (PayloadWriter)
		{
			PayloadWriter->WritePayload(SkillAbility, OutgoingEventData, &InEventScope);
		}
	}

	SkillAbility.ExecuteScopedGameplayEvent(&InEventScope, OutgoingEventData, BindingScope);
}

void UMASkillEventSource::HandleScopedEvent(const FGameplayTag& SourceEventTag, const FGameplayEventData& EventData)
{
	if (!OwnerSkillAbility || !EventScope) return;
	HandleSourceEvent(*OwnerSkillAbility, *EventScope, SourceEventTag, EventData);
}

void UMASkillEventSource::HandleSourceEvent(
	UMASkillAbility& SkillAbility,
	UMASkillModuleInstance& InEventScope,
	const FGameplayTag& SourceEventTag,
	const FGameplayEventData& EventData) const
{
	if (SourceEventTag != EmittedTag) return;
	EmitEvent(SkillAbility, InEventScope, EventData);
}

void UMASkillEventSource::HandleSkillActivated()
{
	StartSource(OwnerSkillAbility);
}

void UMASkillEventSource::HandleSkillDeactivated()
{
	StopSource();
}

