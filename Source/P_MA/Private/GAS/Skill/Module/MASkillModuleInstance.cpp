#include "GAS/Skill/Module/MASkillModuleInstance.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
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

void UMASkillModuleInstance::SetInSkillSlot(bool bInSkillSlot)
{
	if (bIsInSkillSlot == bInSkillSlot) return;

	bIsInSkillSlot = bInSkillSlot;
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
	return GetCooldownRemainingSeconds() > 0.f;
}

float UMASkillModuleInstance::GetCooldownRemainingSeconds() const
{
	return FMath::Max(ModuleCooldownEndTimeSeconds - GetCurrentServerTimeSeconds(), 0.f);
}

void UMASkillModuleInstance::StartCooldown(float DurationSeconds)
{
	if (DurationSeconds <= 0.f || IsCooldownActive()) return;

	ModuleCooldownEndTimeSeconds = GetCurrentServerTimeSeconds() + DurationSeconds;
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

