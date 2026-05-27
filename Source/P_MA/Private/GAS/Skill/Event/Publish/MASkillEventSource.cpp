#include "GAS/Skill/Event/Publish/MASkillEventSource.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadWriter.h"

void UMASkillEventSource::InitializeRuntime(UMASkillAbility* SkillAbility, UMASkillModuleInstance* InEventOwnerScope)
{
	if (!SkillAbility || !InEventOwnerScope) return;
	if (OwnerSkillAbility == SkillAbility && EventOwnerScope == InEventOwnerScope) return;
	if (OwnerSkillAbility) DeinitializeRuntime();

	OwnerSkillAbility = SkillAbility;
	EventOwnerScope = InEventOwnerScope;
	ScopedEventDelegateHandle = EventOwnerScope->OnScopedEvent().AddUObject(this, &UMASkillEventSource::HandleScopedEvent);
	OwnerSkillAbility->OnSkillActivated().AddUObject(this, &UMASkillEventSource::HandleSkillActivated);
	OwnerSkillAbility->OnSkillDeactivated().AddUObject(this, &UMASkillEventSource::HandleSkillDeactivated);
}

void UMASkillEventSource::DeinitializeRuntime()
{
	if (!OwnerSkillAbility) return;

	StopSource();
	if (EventOwnerScope && ScopedEventDelegateHandle.IsValid())
	{
		EventOwnerScope->OnScopedEvent().Remove(ScopedEventDelegateHandle);
	}
	ScopedEventDelegateHandle.Reset();
	OwnerSkillAbility->OnSkillActivated().RemoveAll(this);
	OwnerSkillAbility->OnSkillDeactivated().RemoveAll(this);
	EventOwnerScope = nullptr;
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
	if (!EventOwnerScope) return;

	EmitEvent(*OwnerSkillAbility, *EventOwnerScope, EventData);
}

void UMASkillEventSource::EmitEvent(
	UMASkillAbility& SkillAbility,
	UMASkillModuleInstance& InEventOwnerScope,
	const FGameplayEventData& EventData) const
{
	if (!EmittedTag.IsValid()) return;

	FGameplayEventData OutgoingEventData = EventData;
	OutgoingEventData.EventTag = EmittedTag;

	for (UMASkillPayloadWriter* PayloadWriter : PreEmitPayloadWriters)
	{
		if (PayloadWriter)
		{
			PayloadWriter->WritePayload(SkillAbility, OutgoingEventData, &InEventOwnerScope);
		}
	}

	SkillAbility.ExecuteScopedGameplayEvent(&InEventOwnerScope, OutgoingEventData, RuntimeScope);
}

void UMASkillEventSource::HandleScopedEvent(const FGameplayTag& SourceEventTag, const FGameplayEventData& EventData)
{
	if (!OwnerSkillAbility || !EventOwnerScope) return;
	HandleSourceEvent(*OwnerSkillAbility, *EventOwnerScope, SourceEventTag, EventData);
}

void UMASkillEventSource::HandleSourceEvent(
	UMASkillAbility& SkillAbility,
	UMASkillModuleInstance& InEventOwnerScope,
	const FGameplayTag& SourceEventTag,
	const FGameplayEventData& EventData) const
{
	if (SourceEventTag != EmittedTag) return;
	EmitEvent(SkillAbility, InEventOwnerScope, EventData);
}

void UMASkillEventSource::HandleSkillActivated()
{
	StartSource(OwnerSkillAbility);
}

void UMASkillEventSource::HandleSkillDeactivated()
{
	StopSource();
}

