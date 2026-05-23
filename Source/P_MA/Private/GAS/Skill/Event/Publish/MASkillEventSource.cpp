#include "GAS/Skill/Event/Publish/MASkillEventSource.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadWriter.h"

void UMASkillEventSource::InitializeRuntime(UMASkillAbility* SkillAbility)
{
	if (!SkillAbility || OwnerSkillAbility == SkillAbility) return;
	if (OwnerSkillAbility) DeinitializeRuntime();

	OwnerSkillAbility = SkillAbility;
	OwnerSkillAbility->OnSkillActivated().AddUObject(this, &UMASkillEventSource::HandleSkillActivated);
	OwnerSkillAbility->OnSkillDeactivated().AddUObject(this, &UMASkillEventSource::HandleSkillDeactivated);
}

void UMASkillEventSource::DeinitializeRuntime()
{
	if (!OwnerSkillAbility) return;

	StopSource();
	OwnerSkillAbility->OnSkillActivated().RemoveAll(this);
	OwnerSkillAbility->OnSkillDeactivated().RemoveAll(this);
	OwnerSkillAbility = nullptr;
}

void UMASkillEventSource::EmitEvent() const
{
	FGameplayEventData Payload;
	EmitEvent(Payload);
}

void UMASkillEventSource::EmitEvent(const FGameplayEventData& Payload) const
{
	if (!OwnerSkillAbility || !EmittedTag.IsValid()) return;

	FGameplayEventData EventPayload = Payload;
	EventPayload.EventTag = EmittedTag;

	for (UMASkillPayloadWriter* PayloadWriter : PreEmitPayloadWriters)
	{
		if (PayloadWriter)
		{
			PayloadWriter->WritePayload(*OwnerSkillAbility, EventPayload, RuntimeScope);
		}
	}

	OwnerSkillAbility->SendSkillGameplayEvent(EventPayload, RuntimeScope);
}

void UMASkillEventSource::HandleSkillActivated()
{
	StartSource(OwnerSkillAbility);
}

void UMASkillEventSource::HandleSkillDeactivated()
{
	StopSource();
}

