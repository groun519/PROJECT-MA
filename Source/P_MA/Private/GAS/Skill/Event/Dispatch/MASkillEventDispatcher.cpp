#include "GAS/Skill/Event/Dispatch/MASkillEventDispatcher.h"

#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillEventDispatcher::Refresh(const TArray<FMASkillSlotRuntimeState>& SkillSlotRuntimeStates)
{
	Clear();

	const UMASkillManagerComponent* SkillManager = CastChecked<UMASkillManagerComponent>(GetOuter());
	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		UMASkillModuleInstance* AssembledModuleInstance = SlotState.AssembledModuleInstance;
		const UMASkillDefinition* Definition = AssembledModuleInstance
			? AssembledModuleInstance->GetDefinition()
			: nullptr;
		UMASkillAbility* ExecutorAbility = SkillManager->GetSkillAbility(SlotState.SlotTag);
		if (Definition && ExecutorAbility)
		{
			for (const FMASkillEventBinding& Binding : Definition->GetEventBindings())
			{
				if (!Binding.EventTag.IsValid()) continue;

				FMASkillRegisteredEventBinding& RegisteredBinding =
					BindingsByEventTag.FindOrAdd(Binding.EventTag).Values.AddDefaulted_GetRef();
				RegisteredBinding.Binding = Binding;
				RegisteredBinding.ExecutorAbility = ExecutorAbility;
			}
		}

		if (!AssembledModuleInstance) continue;
		for (UMASkillModuleInstance* ModuleInstance : SlotState.SourceModuleInstances)
		{
			if (!ModuleInstance || !ModuleInstance->IsActive()) continue;

			ModuleInstance->ForEachAddon([&](const UMASkillModuleAddon& Addon)
			{
				Addon.RegisterEventSubscriptions(
					*this,
					*ModuleInstance,
					*AssembledModuleInstance);
			});
		}
	}
}

void UMASkillEventDispatcher::Subscribe(
	FGameplayTag EventTag,
	const FMASkillEventEvaluatedSignature::FDelegate& Listener)
{
	if (!EventTag.IsValid() || !Listener.IsBound()) return;
	EventEvaluatedDelegates.FindOrAdd(EventTag).Add(Listener);
}

void UMASkillEventDispatcher::Clear()
{
	BindingsByEventTag.Reset();
	EventEvaluatedDelegates.Reset();
}

void UMASkillEventDispatcher::Dispatch(
	const FMASkillEvent& Event,
	UMASkillAbility* ExecutorAbility)
{
	DispatchGroup(MakeArrayView(&Event, 1), ExecutorAbility);
}

void UMASkillEventDispatcher::DispatchGroup(
	TConstArrayView<FMASkillEvent> Events,
	UMASkillAbility* ExecutorAbility)
{
	if (Events.IsEmpty()) return;

	const FMASkillEvent& FirstEvent = Events[0];
	for (int32 EventIndex = 1; EventIndex < Events.Num(); ++EventIndex)
	{
		const FMASkillEvent& Event = Events[EventIndex];
		if (!ensureMsgf(
			Event.Tag == FirstEvent.Tag
			&& Event.SourceScopes.Module == FirstEvent.SourceScopes.Module
			&& Event.SourceScopes.Skill == FirstEvent.SourceScopes.Skill,
			TEXT("Skill event groups require matching tags and source scopes.")))
		{
			return;
		}
	}

	struct FPendingEventAction
	{
		int32 EventIndex = INDEX_NONE;
		TObjectPtr<UMASkillAction> Action;
		TWeakObjectPtr<UMASkillAbility> Ability;
		FMASkillScopes Scopes;
	};

	TArray<FPendingEventAction> PendingActions;
	const FMASkillRegisteredEventBindings* RegisteredBindings = BindingsByEventTag.Find(FirstEvent.Tag);
	for (int32 EventIndex = 0; EventIndex < Events.Num(); ++EventIndex)
	{
		const FMASkillEvent& Event = Events[EventIndex];
		if (!RegisteredBindings) continue;
		for (const FMASkillRegisteredEventBinding& RegisteredBinding : RegisteredBindings->Values)
		{
			const FMASkillEventBinding& Binding = RegisteredBinding.Binding;
			if (!Binding.CanExecute() || !Binding.Action) continue;

			FMASkillScopes ActionScopes;
			if (!Binding.TryResolveScopes(Event.SourceScopes, ActionScopes)) continue;

			FPendingEventAction& PendingAction = PendingActions.AddDefaulted_GetRef();
			PendingAction.EventIndex = EventIndex;
			PendingAction.Action = Binding.Action;
			PendingAction.Ability = RegisteredBinding.ExecutorAbility;
			PendingAction.Scopes = ActionScopes;
		}
	}

	if (FMASkillEventEvaluatedSignature* EvaluatedDelegate = EventEvaluatedDelegates.Find(FirstEvent.Tag))
	{
		EvaluatedDelegate->Broadcast(FirstEvent);
	}

	for (const FPendingEventAction& PendingAction : PendingActions)
	{
		UMASkillAbility* BindingAbility = PendingAction.Ability.Get();
		if (!BindingAbility
			|| PendingAction.EventIndex < 0
			|| PendingAction.EventIndex >= Events.Num())
		{
			continue;
		}

		PendingAction.Action->Execute(
			*BindingAbility,
			Events[PendingAction.EventIndex],
			PendingAction.Scopes);
	}
}
