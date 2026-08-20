#include "GAS/Skill/Module/MASkillModuleInstance.h"

#include "GAS/Skill/Module/MASkillModule.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

UMASkillModuleInstance::UMASkillModuleInstance() = default;

void UMASkillModuleInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMASkillModuleInstance, ModuleGroup);
	DOREPLIFETIME(UMASkillModuleInstance, AddonRuntimeData);
	DOREPLIFETIME(UMASkillModuleInstance, ModuleCooldownEndTimeSeconds);
}

bool UMASkillModuleInstance::SetRootModule(UMASkillModule* InRootModule)
{
	if (InRootModule && InRootModule->GetModuleType() != EMASkillModuleType::Module)
	{
		return false;
	}

	ComposedModule.Reset(*this);
	ModuleGroup.RootModule = InRootModule;
	AddonRuntimeData.Reset();
	if (ModuleGroup.RootModule)
	{
		ModuleGroup.RootModule->InitializeAddonRuntimeData(AddonRuntimeData);
	}
	ResetPayloads();
	return true;
}

void UMASkillModuleInstance::OnRep_ModuleGroup(FMASkillModuleGroup& PreviousModuleGroup)
{
	ComposedModule.Reset(*this);
	if (ModuleGroup.RootModule != PreviousModuleGroup.RootModule
		&& ModuleGroup.RootModule)
	{
		ModuleGroup.RootModule->InitializeAddonRuntimeData(AddonRuntimeData);
	}
	ResetPayloads();
	if (ModuleGroup.SubModules != PreviousModuleGroup.SubModules)
	{
		OnSubModulesChanged.Broadcast(this);
	}
	OnStateChanged.Broadcast();
}

bool UMASkillModuleInstance::SetSubModuleAt(
	const int32 SubModuleIndex,
	UMASkillModule* SubModule)
{
	const AActor* OwnerActor = GetTypedOuter<AActor>();
	if (OwnerActor && !OwnerActor->HasAuthority()) return false;
	if (SubModuleIndex < 0) return false;
	if (SubModule && SubModule->GetModuleType() != EMASkillModuleType::Sub) return false;

	UMASkillModule* CurrentSubModule = ModuleGroup.SubModules.IsValidIndex(SubModuleIndex)
		? ModuleGroup.SubModules[SubModuleIndex].Get()
		: nullptr;
	if (CurrentSubModule == SubModule) return true;

	if (ModuleGroup.SubModules.Num() <= SubModuleIndex)
	{
		ModuleGroup.SubModules.SetNum(SubModuleIndex + 1);
	}
	ModuleGroup.SubModules[SubModuleIndex] = SubModule;

	int32 NewNum = ModuleGroup.SubModules.Num();
	while (NewNum > 0 && !ModuleGroup.SubModules[NewNum - 1]) --NewNum;
	ModuleGroup.SubModules.SetNum(NewNum);

	ComposedModule.Reset(*this);
	ResetPayloads();
	OnSubModulesChanged.Broadcast(this);
	OnStateChanged.Broadcast();
	if (AActor* MutableOwnerActor = GetTypedOuter<AActor>())
	{
		MutableOwnerActor->ForceNetUpdate();
	}
	return true;
}

const UMASkillModule* UMASkillModuleInstance::ResolveComposedModule()
{
	return ComposedModule.Resolve(*this, ModuleGroup);
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

void UMASkillModuleInstance::ResetPayloads()
{
	PayloadStore.Reset();
	if (ModuleGroup.RootModule)
	{
		ModuleGroup.RootModule->ApplyPayloadsTo(PayloadStore);
	}
	for (const UMASkillModule* SubModule : ModuleGroup.SubModules)
	{
		if (SubModule) SubModule->ApplyPayloadsTo(PayloadStore);
	}
	RefreshAddonPayloadMirrors();
}

void UMASkillModuleInstance::RefreshAddonPayloadMirrors()
{
	if (!ModuleGroup.RootModule) return;
	ModuleGroup.RootModule->ApplyAddonPayloadMirrors(AddonRuntimeData, PayloadStore);
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

