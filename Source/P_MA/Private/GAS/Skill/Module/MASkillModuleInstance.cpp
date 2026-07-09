#include "GAS/Skill/Module/MASkillModuleInstance.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Dispatch/MASkillEventDispatcher.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

UMASkillModuleInstance::UMASkillModuleInstance() = default;

void UMASkillModuleInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMASkillModuleInstance, Definition);
	DOREPLIFETIME(UMASkillModuleInstance, AddonRuntimeData);
	DOREPLIFETIME(UMASkillModuleInstance, ModuleCooldownEndTimeSeconds);
}

void UMASkillModuleInstance::SetDefinition(UMASkillDefinition* InDefinition)
{
	Definition = InDefinition;
	AddonRuntimeData.Reset();
	if (Definition) Definition->InitializeAddonRuntimeData(AddonRuntimeData);
	InitializePayloadStore();
}

void UMASkillModuleInstance::OnRep_Definition()
{
	if (Definition) Definition->InitializeAddonRuntimeData(AddonRuntimeData);
	InitializePayloadStore();
	OnStateChanged.Broadcast();
}

void UMASkillModuleInstance::OnRep_AddonRuntimeData()
{
	RefreshAddonPayloadMirrors();
	OnStateChanged.Broadcast();
}

bool UMASkillModuleInstance::CanModifyAddonRuntimeData() const
{
	const AActor* OwnerActor = GetTypedOuter<AActor>();
	return !OwnerActor || OwnerActor->HasAuthority();
}

void UMASkillModuleInstance::NotifyAddonRuntimeDataChanged()
{
	RefreshAddonPayloadMirrors();
	OnStateChanged.Broadcast();

	AActor* OwnerActor = GetTypedOuter<AActor>();
	if (OwnerActor)
	{
		OwnerActor->ForceNetUpdate();
	}
}

void UMASkillModuleInstance::InitializePayloadStore()
{
	PayloadStore.Reset();
	if (Definition) Definition->ApplyPayloadsTo(PayloadStore);
	RefreshAddonPayloadMirrors();
}

void UMASkillModuleInstance::RefreshAddonPayloadMirrors()
{
	if (!Definition) return;
	Definition->ApplyAddonPayloadMirrors(AddonRuntimeData, PayloadStore);
}

bool UMASkillModuleInstance::IsCooldownActive() const
{
	return GetCurrentServerTimeSeconds() < ModuleCooldownEndTimeSeconds;
}

void UMASkillModuleInstance::RegisterCooldownEvents(
	UMASkillEventDispatcher& EventDispatcher,
	UMASkillModuleInstance* SkillScope)
{
	if (!IsActive() || !Definition || !SkillScope) return;

	const FMASkillModuleCooldownConfig& CooldownConfig = Definition->GetModuleCooldownConfig();
	if (CooldownConfig.DurationSeconds <= 0.f || CooldownConfig.TriggerEventTags.IsEmpty()) return;

	const TWeakObjectPtr<UMASkillModuleInstance> WeakSkillScope = SkillScope;
	for (const FGameplayTag& EventTag : CooldownConfig.TriggerEventTags)
	{
		EventDispatcher.AddEventEvaluatedListener(
			EventTag,
			FMASkillEventEvaluatedSignature::FDelegate::CreateUObject(
				this,
				&UMASkillModuleInstance::HandleCooldownEvent,
				WeakSkillScope));
	}
}

void UMASkillModuleInstance::HandleCooldownEvent(
	const FMASkillEvent& Event,
	TWeakObjectPtr<UMASkillModuleInstance> SkillScope)
{
	if (!Definition || !IsActive() || IsCooldownActive()) return;

	const FMASkillModuleCooldownConfig& CooldownConfig = Definition->GetModuleCooldownConfig();
	switch (CooldownConfig.BindingScope)
	{
	case EMASkillEventBindingScope::Module:
		if (Event.SourceScopes.Module != this) return;
		break;

	case EMASkillEventBindingScope::Skill:
		if (Event.SourceScopes.Skill != SkillScope.Get()) return;
		break;

	case EMASkillEventBindingScope::Global:
		break;
	}

	ModuleCooldownEndTimeSeconds = GetCurrentServerTimeSeconds() + CooldownConfig.DurationSeconds;
	RefreshModuleCooldownState();

	if (AActor* OwnerActor = GetTypedOuter<AActor>())
	{
		OwnerActor->ForceNetUpdate();
	}
}

void UMASkillModuleInstance::SetActive(
	bool bInActive,
	const FGameplayTag& InInactiveReasonTag)
{
	const FGameplayTag NewInactiveReasonTag = bInActive ? FGameplayTag() : InInactiveReasonTag;
	if (bIsActive == bInActive && InactiveReasonTag == NewInactiveReasonTag) return;

	bIsActive = bInActive;
	InactiveReasonTag = NewInactiveReasonTag;
	OnStateChanged.Broadcast();
}

void UMASkillModuleInstance::RefreshModuleCooldownState()
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(ModuleCooldownTimerHandle);
	const float RemainingSeconds = ModuleCooldownEndTimeSeconds - GetCurrentServerTimeSeconds();
	if (RemainingSeconds > 0.f)
	{
		World->GetTimerManager().SetTimer(
			ModuleCooldownTimerHandle,
			this,
			&UMASkillModuleInstance::RefreshModuleCooldownState,
			RemainingSeconds,
			false);
	}

	OnStateChanged.Broadcast();
}
float UMASkillModuleInstance::GetCurrentServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	if (!World) return 0.f;

	const AGameStateBase* GameState = World->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : World->GetTimeSeconds();
}

