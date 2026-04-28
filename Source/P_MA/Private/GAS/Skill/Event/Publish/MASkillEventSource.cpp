#include "GAS/Skill/Event/Publish/MASkillEventSource.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Skill/MASkillAbility.h"

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
	if (!OwnerSkillAbility || !EmittedTag.IsValid()) return;

	FGameplayEventData Payload;
	Payload.EventTag = EmittedTag;
	OwnerSkillAbility->SendSkillGameplayEvent(Payload, RuntimeScope);
}

void UMASkillEventSource::HandleSkillActivated()
{
	StartSource(OwnerSkillAbility);
}

void UMASkillEventSource::HandleSkillDeactivated()
{
	StopSource();
}

