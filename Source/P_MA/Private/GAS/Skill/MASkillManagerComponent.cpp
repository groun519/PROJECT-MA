#include "GAS/Skill/MASkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"
#include "GAS/Skill/Definition/MASkillAssembler.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Dispatch/MASkillEventDispatcher.h"
#include "GAS/Skill/Event/Routing/MASkillEventRouter.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "Net/UnrealNetwork.h"

UMASkillManagerComponent::UMASkillManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UMASkillManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Dispatcher = NewObject<UMASkillEventDispatcher>(this, TEXT("EventDispatcher"));
	check(Dispatcher);
	Router = NewObject<UMASkillEventRouter>(this, TEXT("EventRouter"));
	check(Router);
}

void UMASkillManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	check(Router);
	Router->Clear();
	check(Dispatcher);
	Dispatcher->Clear();
	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		if (UMASkillRuntimeRegistry* RuntimeRegistry = SlotState.AssembledModuleInstance
			? SlotState.AssembledModuleInstance->GetRuntimeRegistry()
			: nullptr)
		{
			RuntimeRegistry->Cleanup();
		}
	}
	Super::EndPlay(EndPlayReason);
}

void UMASkillManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMASkillManagerComponent, ReplicatedSkillSlotRuntimeStates, COND_OwnerOnly);
	DOREPLIFETIME(UMASkillManagerComponent, ActivePreviewVisualElementTag);
}

void UMASkillManagerComponent::Multicast_SpawnSkillAreaImpact_Implementation(
	FMASkillWorldAreaShape Area,
	FGameplayTag ElementSourceTag)
{
	SpawnSkillAreaImpactLocal(Area, ElementSourceTag, false);
}

void UMASkillManagerComponent::Multicast_SpawnPredictedSkillAreaImpact_Implementation(
	FMASkillWorldAreaShape Area,
	FGameplayTag ElementSourceTag)
{
	SpawnSkillAreaImpactLocal(Area, ElementSourceTag, true);
}

void UMASkillManagerComponent::SpawnSkillAreaImpactLocal(
	const FMASkillWorldAreaShape& Area,
	FGameplayTag ElementSourceTag,
	bool bSkipAutonomousProxy)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || OwnerActor->GetNetMode() == NM_DedicatedServer) return;
	if (bSkipAutonomousProxy && OwnerActor->GetLocalRole() == ROLE_AutonomousProxy) return;

	MASkillAreaDecalStatics::SpawnImpactLocal(OwnerActor, ElementSourceTag, Area);
}

void UMASkillManagerComponent::InitializeGrantedAbilities()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	for (const FGameplayTag& SlotTag : GatherUniqueSkillSlotTags())
	{
		RebuildSkill(SlotTag);
	}
}

void UMASkillManagerComponent::PrepareSkillSlotRuntimeStatesForUI()
{
	const TArray<FGameplayTag> SlotTags = GatherUniqueSkillSlotTags();
	SkillSlotRuntimeStates.Reserve(SlotTags.Num() + 1);
	FindOrAddSlotRuntimeState(FMASkillSystemStatics::GetPassiveSlotTag());
	for (const FGameplayTag& SlotTag : SlotTags)
	{
		FindOrAddSlotRuntimeState(SlotTag);
	}
}

/** Module Lifetime **/
UMASkillModuleInstance* UMASkillManagerComponent::CreateModuleInstance(UMASkillDefinition* Definition)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority() || !Definition) return nullptr;

	UMASkillModuleInstance* ModuleInstance = NewObject<UMASkillModuleInstance>(OwnerActor);
	ModuleInstance->SetDefinition(Definition);
	OwnerActor->AddReplicatedSubObject(ModuleInstance, COND_OwnerOnly);
	return ModuleInstance;
}

void UMASkillManagerComponent::UnregisterModuleInstance(UMASkillModuleInstance* ModuleInstance)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !ModuleInstance) return;

	OwnerActor->RemoveReplicatedSubObject(ModuleInstance);
}

/** Slot Composition **/
bool UMASkillManagerComponent::ReplaceDefinitionAt(
	FGameplayTag SlotTag,
	int32 ModuleIndex,
	UMASkillDefinition* NewDefinition)
{
	if (!CanMutateSkillSlots()) return false;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return false;
	if (!IsValidModuleSlotIndex(SlotTag, ModuleIndex)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotTag, SlotState.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstance = SlotState.SourceModuleInstances[ModuleIndex];
	UMASkillModuleInstance* NewModuleInstance = NewDefinition
		? CreateModuleInstance(NewDefinition)
		: nullptr;
	if (NewDefinition && !NewModuleInstance) return false;
	SlotState.SourceModuleInstances[ModuleIndex] = NewModuleInstance;

	if (!RebuildSkill(SlotTag))
	{
		SlotState.SourceModuleInstances[ModuleIndex] = PreviousModuleInstance;
		UnregisterModuleInstance(NewModuleInstance);
		RebuildSkill(SlotTag);
		return false;
	}

	UnregisterModuleInstance(PreviousModuleInstance);
	return true;
}

bool UMASkillManagerComponent::ReplaceDefinitionsAt(
	FGameplayTag SlotTag,
	const TArray<TObjectPtr<UMASkillDefinition>>& NewDefinitions)
{
	if (!CanMutateSkillSlots()) return false;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return false;
	const int32 ModuleSlotCount = GetModuleSlotCount(SlotTag);

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotTag, SlotState.SourceModuleInstances);

	TArray<TObjectPtr<UMASkillModuleInstance>> PreviousModuleInstances = SlotState.SourceModuleInstances;
	TArray<TObjectPtr<UMASkillModuleInstance>> NewModuleInstances;
	NewModuleInstances.SetNum(ModuleSlotCount);
	for (int32 Index = 0; Index < ModuleSlotCount; ++Index)
	{
		UMASkillDefinition* Definition = NewDefinitions.IsValidIndex(Index)
			? NewDefinitions[Index].Get()
			: nullptr;
		if (!Definition) continue;

		NewModuleInstances[Index] = CreateModuleInstance(Definition);
		if (NewModuleInstances[Index]) continue;

		for (UMASkillModuleInstance* CreatedModuleInstance : NewModuleInstances)
		{
			UnregisterModuleInstance(CreatedModuleInstance);
		}
		return false;
	}
	SlotState.SourceModuleInstances = NewModuleInstances;

	if (!RebuildSkill(SlotTag))
	{
		SlotState.SourceModuleInstances = PreviousModuleInstances;
		for (UMASkillModuleInstance* NewModuleInstance : NewModuleInstances)
		{
			UnregisterModuleInstance(NewModuleInstance);
		}
		RebuildSkill(SlotTag);
		return false;
	}

	for (UMASkillModuleInstance* PreviousModuleInstance : PreviousModuleInstances)
	{
		UnregisterModuleInstance(PreviousModuleInstance);
	}
	return true;
}

bool UMASkillManagerComponent::ReplaceModuleInstanceAt(
	FGameplayTag SlotTag,
	int32 ModuleIndex,
	UMASkillModuleInstance* NewModuleInstance,
	UMASkillModuleInstance*& OutPreviousModuleInstance)
{
	OutPreviousModuleInstance = nullptr;

	if (!CanMutateSkillSlots()) return false;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return false;
	if (!IsValidModuleSlotIndex(SlotTag, ModuleIndex)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotTag, SlotState.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstance = SlotState.SourceModuleInstances[ModuleIndex];
	SlotState.SourceModuleInstances[ModuleIndex] = NewModuleInstance;

	if (!RebuildSkill(SlotTag))
	{
		SlotState.SourceModuleInstances[ModuleIndex] = PreviousModuleInstance;
		RebuildSkill(SlotTag);
		return false;
	}

	OutPreviousModuleInstance = PreviousModuleInstance;
	return true;
}

bool UMASkillManagerComponent::RequestSwapModuleSlotsBetween(
	FGameplayTag SlotTagA,
	int32 IndexA,
	FGameplayTag SlotTagB,
	int32 IndexB)
{
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTagA)
		|| !FMASkillSystemStatics::IsSkillSlotTag(SlotTagB))
	{
		return false;
	}
	if (!IsValidModuleSlotIndex(SlotTagA, IndexA) || !IsValidModuleSlotIndex(SlotTagB, IndexB)) return false;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	if (OwnerActor->HasAuthority())
	{
		return SwapModuleSlotsBetween(SlotTagA, IndexA, SlotTagB, IndexB);
	}

	ServerSwapModuleSlotsBetween(SlotTagA, IndexA, SlotTagB, IndexB);
	return true;
}

bool UMASkillManagerComponent::RequestMoveModuleSlot(
	const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
	int32 SourceIndex,
	UActorComponent* TargetOwner,
	const TArray<TObjectPtr<UMASkillModuleInstance>>* TargetSlots,
	int32 TargetIndex)
{
	if (!SourceSlots || !TargetOwner || !TargetSlots || TargetIndex == INDEX_NONE) return false;

	FGameplayTag SourceSlotTag;
	if (!FindSlotTagForModuleSlots(SourceSlots, SourceSlotTag)) return false;
	if (!IsValidModuleSlotIndex(SourceSlotTag, SourceIndex)) return false;
	if (!(*SourceSlots)[SourceIndex] || !(*SourceSlots)[SourceIndex]->IsValid()) return false;

	if (UMASkillManagerComponent* TargetSkillManager = Cast<UMASkillManagerComponent>(TargetOwner))
	{
		if (TargetSkillManager != this) return false;

		FGameplayTag TargetSlotTag;
		if (!FindSlotTagForModuleSlots(TargetSlots, TargetSlotTag)) return false;

		return RequestSwapModuleSlotsBetween(SourceSlotTag, SourceIndex, TargetSlotTag, TargetIndex);
	}

	if (UMASkillModuleInventoryComponent* TargetInventory = Cast<UMASkillModuleInventoryComponent>(TargetOwner))
	{
		const TArray<TObjectPtr<UMASkillModuleInstance>>* InventorySlots = TargetInventory->GetModuleSlotsForUI();
		if (TargetSlots != InventorySlots) return false;

		return TargetInventory->RequestMoveSkillSlotToInventorySlot(SourceSlotTag, SourceIndex, TargetIndex);
	}

	return false;
}

bool UMASkillManagerComponent::SwapModuleSlotsBetween(
	FGameplayTag SlotTagA,
	int32 IndexA,
	FGameplayTag SlotTagB,
	int32 IndexB)
{
	if (!CanMutateSkillSlots()) return false;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTagA)
		|| !FMASkillSystemStatics::IsSkillSlotTag(SlotTagB))
	{
		return false;
	}
	if (!IsValidModuleSlotIndex(SlotTagA, IndexA) || !IsValidModuleSlotIndex(SlotTagB, IndexB)) return false;
	if (SlotTagA == SlotTagB && IndexA == IndexB) return true;

	FMASkillSlotRuntimeState& SlotStateA = FindOrAddSlotRuntimeState(SlotTagA);
	FMASkillSlotRuntimeState& SlotStateB = FindOrAddSlotRuntimeState(SlotTagB);

	NormalizeModuleInstanceSlots(SlotTagA, SlotStateA.SourceModuleInstances);
	NormalizeModuleInstanceSlots(SlotTagB, SlotStateB.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstanceA = SlotStateA.SourceModuleInstances[IndexA];
	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstanceB = SlotStateB.SourceModuleInstances[IndexB];
	Swap(SlotStateA.SourceModuleInstances[IndexA], SlotStateB.SourceModuleInstances[IndexB]);

	if (SlotTagA == SlotTagB)
	{
		if (RebuildSkill(SlotTagA)) return true;

		SlotStateA.SourceModuleInstances[IndexA] = PreviousModuleInstanceA;
		SlotStateB.SourceModuleInstances[IndexB] = PreviousModuleInstanceB;
		RebuildSkill(SlotTagA);
		return false;
	}

	const bool bRebuiltA = RebuildSkill(SlotTagA);
	const bool bRebuiltB = RebuildSkill(SlotTagB);
	if (bRebuiltA && bRebuiltB) return true;

	SlotStateA.SourceModuleInstances[IndexA] = PreviousModuleInstanceA;
	SlotStateB.SourceModuleInstances[IndexB] = PreviousModuleInstanceB;
	RebuildSkill(SlotTagA);
	RebuildSkill(SlotTagB);
	return false;
}

void UMASkillManagerComponent::ServerSwapModuleSlotsBetween_Implementation(
	FGameplayTag SlotTagA,
	int32 IndexA,
	FGameplayTag SlotTagB,
	int32 IndexB)
{
	SwapModuleSlotsBetween(SlotTagA, IndexA, SlotTagB, IndexB);
}

const TArray<TObjectPtr<UMASkillModuleInstance>>* UMASkillManagerComponent::GetModuleSlotsForUI(FGameplayTag SlotTag)
{
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return nullptr;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotTag, SlotState.SourceModuleInstances);
	return &SlotState.SourceModuleInstances;
}

bool UMASkillManagerComponent::FindSlotTagForModuleSlots(
	const TArray<TObjectPtr<UMASkillModuleInstance>>* ModuleSlots,
	FGameplayTag& OutSlotTag) const
{
	OutSlotTag = FGameplayTag();
	if (!ModuleSlots) return false;

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		if (&SlotState.SourceModuleInstances != ModuleSlots) continue;

		OutSlotTag = SlotState.SlotTag;
		return true;
	}

	return false;
}

UMASkillDefinition* UMASkillManagerComponent::GetAssembledDefinition(FGameplayTag SlotTag) const
{
	const FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(SlotTag);
	const UMASkillModuleInstance* AssembledModuleInstance = SlotState ? SlotState->AssembledModuleInstance : nullptr;
	return AssembledModuleInstance ? AssembledModuleInstance->GetDefinition() : nullptr;
}

/** Slot Runtime **/
bool UMASkillManagerComponent::RebuildSkill(FGameplayTag SlotTag)
{
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	UMASkillModuleInstance* PreviousAssembledModuleInstance = SlotState.AssembledModuleInstance;
	if (UMASkillAbility* SkillAbility = ResolveSkillAbility(SlotState);
		SkillAbility && SkillAbility->IsActive())
	{
		SkillAbility->EndSkill();
	}

	SlotState.AssembledModuleInstance = FMASkillAssembler::Assemble(this, SlotTag, SlotState.SourceModuleInstances);
	if (FMASkillSystemStatics::IsActiveSkillSlotTag(SlotTag) || SlotState.AssembledModuleInstance)
	{
		EnsureAbilityForSlot(SlotState);
	}
	RefreshAbilityDefinition(SlotState);
	UpdateReplicatedSkillSlotRuntimeState(SlotState);
	check(Dispatcher);
	Dispatcher->Refresh(SkillSlotRuntimeStates);
	check(Router);
	Router->Refresh(SkillSlotRuntimeStates);
	if (UMASkillRuntimeRegistry* PreviousRuntimeRegistry = PreviousAssembledModuleInstance
		? PreviousAssembledModuleInstance->GetRuntimeRegistry()
		: nullptr)
	{
		PreviousRuntimeRegistry->Cleanup();
	}
	OnSkillSlotChanged.Broadcast(SlotTag);
	return SlotState.AssembledModuleInstance != nullptr
		|| !SlotState.SourceModuleInstances.ContainsByPredicate([](const UMASkillModuleInstance* ModuleInstance)
		{
			return ModuleInstance && ModuleInstance->IsValid() && ModuleInstance->IsActive();
		});
}

void UMASkillManagerComponent::RegisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass)
{
	if (AbilityClass != UMASkillAbility::StaticClass()) return;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	SlotState.AbilityHandle = AbilityHandle;

	if (!SlotState.AssembledModuleInstance)
	{
		RebuildSkill(SlotTag);
		return;
	}

	RefreshAbilityDefinition(SlotState);
	check(Dispatcher);
	Dispatcher->Refresh(SkillSlotRuntimeStates);
}

void UMASkillManagerComponent::UnregisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle)
{
	FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(SlotTag);
	if (!SlotState) return;
	if (SlotState->AbilityHandle != AbilityHandle) return;
	SlotState->AbilityHandle = FGameplayAbilitySpecHandle();
	check(Dispatcher);
	Dispatcher->Refresh(SkillSlotRuntimeStates);
}

bool UMASkillManagerComponent::TryActivateSkill(FGameplayTag SlotTag)
{
	if (!FMASkillSystemStatics::IsActiveSkillSlotTag(SlotTag)) return false;
	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	if (!EnsureAbilityForSlot(SlotState)) return false;
	if (!SlotState.AbilityHandle.IsValid()) return false;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner());
	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner ? AbilitySystemOwner->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent) return false;

	SetActivePreviewVisualElementTagFromSlot(SlotState);
	if (const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SlotState.AbilityHandle))
	{
		if (AbilitySpec->IsActive()) return true;
	}
	if (AbilitySystemComponent->TryActivateAbility(SlotState.AbilityHandle)) return true;

	ClearActivePreviewVisualElementTag();
	return false;
}

UMASkillAbility* UMASkillManagerComponent::GetSkillAbility(FGameplayTag SlotTag) const
{
	const FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(SlotTag);
	return SlotState ? ResolveSkillAbility(*SlotState) : nullptr;
}

void UMASkillManagerComponent::SetActivePreviewVisualElementTagFromSlot(const FMASkillSlotRuntimeState& SlotState)
{
	const UMASkillDefinition* SkillDefinition = SlotState.AssembledModuleInstance
		? SlotState.AssembledModuleInstance->GetDefinition()
		: nullptr;
	ActivePreviewVisualElementTag = SkillDefinition
		? SkillDefinition->GetVisualElementTag()
		: FGameplayTag();
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
}

void UMASkillManagerComponent::ClearActivePreviewVisualElementTag()
{
	if (!ActivePreviewVisualElementTag.IsValid()) return;

	ActivePreviewVisualElementTag = FGameplayTag();
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
}

/** Slot Composition **/
FMASkillSlotRuntimeState* UMASkillManagerComponent::FindSlotRuntimeState(FGameplayTag SlotTag)
{
	return SkillSlotRuntimeStates.FindByPredicate([SlotTag](const FMASkillSlotRuntimeState& SlotState)
	{
		return SlotState.SlotTag == SlotTag;
	});
}

const FMASkillSlotRuntimeState* UMASkillManagerComponent::FindSlotRuntimeState(FGameplayTag SlotTag) const
{
	return SkillSlotRuntimeStates.FindByPredicate([SlotTag](const FMASkillSlotRuntimeState& SlotState)
	{
		return SlotState.SlotTag == SlotTag;
	});
}

FMASkillSlotRuntimeState& UMASkillManagerComponent::FindOrAddSlotRuntimeState(FGameplayTag SlotTag)
{
	if (FMASkillSlotRuntimeState* ExistingState = FindSlotRuntimeState(SlotTag))
	{
		NormalizeModuleInstanceSlots(SlotTag, ExistingState->SourceModuleInstances);
		return *ExistingState;
	}

	// UI socket widgets may keep direct pointers to SourceModuleInstances arrays.
	// Add all runtime states before binding UI, then mutate only array values while widgets are alive.
	ensureMsgf(
		!OnSkillSlotChanged.IsBound(),
		TEXT("Do not add skill slot runtime states while UI widgets may hold direct slot array pointers."));
	FMASkillSlotRuntimeState& NewState = SkillSlotRuntimeStates.AddDefaulted_GetRef();
	NewState.SlotTag = SlotTag;
	NormalizeModuleInstanceSlots(SlotTag, NewState.SourceModuleInstances);
	return NewState;
}

int32 UMASkillManagerComponent::GetModuleSlotCount(FGameplayTag SlotTag)
{
	return FMASkillSystemStatics::IsPassiveSkillSlotTag(SlotTag)
		? PassiveModuleSlotCount
		: ActiveModuleSlotCount;
}

bool UMASkillManagerComponent::IsValidModuleSlotIndex(FGameplayTag SlotTag, int32 Index)
{
	return Index >= 0 && Index < GetModuleSlotCount(SlotTag);
}

void UMASkillManagerComponent::NormalizeModuleInstanceSlots(
	FGameplayTag SlotTag,
	TArray<TObjectPtr<UMASkillModuleInstance>>& ModuleInstances)
{
	const int32 ModuleSlotCount = GetModuleSlotCount(SlotTag);
	if (ModuleInstances.Num() != 0 && ModuleInstances.Num() != ModuleSlotCount)
	{
		ensureMsgf(
			false,
			TEXT("Do not resize skill module slots while UI widgets may hold direct slot array pointers."));
	}
	ModuleInstances.SetNum(ModuleSlotCount);
}

TArray<FGameplayTag> UMASkillManagerComponent::GatherUniqueSkillSlotTags() const
{
	TArray<FGameplayTag> UniqueSlotTags;
	TSet<FGameplayTag> SeenSlotTags;

	for (const FMASkillSlotStack& SkillSlotStack : SkillSlotStacks)
	{
		const FGameplayTag& SlotTag = SkillSlotStack.SlotTag;
		if (!FMASkillSystemStatics::IsActiveSkillSlotTag(SlotTag)) continue;

		if (SeenSlotTags.Contains(SlotTag))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("UMASkillManagerComponent ignored duplicate SkillSlotStacks entry for SlotTag %s on %s."),
				*SlotTag.ToString(),
				*GetNameSafe(GetOwner()));
			continue;
		}

		SeenSlotTags.Add(SlotTag);
		UniqueSlotTags.Add(SlotTag);
	}

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		if (!FMASkillSystemStatics::IsActiveSkillSlotTag(SlotState.SlotTag)
			|| SeenSlotTags.Contains(SlotState.SlotTag))
		{
			continue;
		}

		SeenSlotTags.Add(SlotState.SlotTag);
		UniqueSlotTags.Add(SlotState.SlotTag);
	}

	return UniqueSlotTags;
}

bool UMASkillManagerComponent::CanMutateSkillSlots() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

/** Slot Runtime **/
bool UMASkillManagerComponent::EnsureAbilityForSlot(FMASkillSlotRuntimeState& SlotState)
{
	if (SlotState.AbilityHandle.IsValid()) return true;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return true;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(OwnerActor);
	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner ? AbilitySystemOwner->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent) return false;

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability || AbilitySpec.Ability->GetClass() != UMASkillAbility::StaticClass()) continue;
		if (!AbilitySpec.DynamicAbilityTags.HasTagExact(SlotState.SlotTag)) continue;

		SlotState.AbilityHandle = AbilitySpec.Handle;
		return true;
	}

	const int32 SlotInputID = FMASkillSystemStatics::ResolveSlotInputID(SlotState.SlotTag);
	FGameplayAbilitySpec AbilitySpec(UMASkillAbility::StaticClass(), 1, SlotInputID, nullptr);
	AbilitySpec.DynamicAbilityTags.AddTag(SlotState.SlotTag);
	SlotState.AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	return SlotState.AbilityHandle.IsValid();
}

/** Slot Replication **/
void UMASkillManagerComponent::OnRep_ReplicatedSkillSlotRuntimeStates()
{
	ApplyReplicatedSkillSlotRuntimeStates();
}

void UMASkillManagerComponent::ApplyReplicatedSkillSlotRuntimeStates()
{
	for (const FMASkillReplicatedSlotRuntimeState& ReplicatedSlotState : ReplicatedSkillSlotRuntimeStates)
	{
		if (!FMASkillSystemStatics::IsSkillSlotTag(ReplicatedSlotState.SlotTag)) continue;

		FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(ReplicatedSlotState.SlotTag);
		const int32 ModuleSlotCount = GetModuleSlotCount(ReplicatedSlotState.SlotTag);
		SlotState.SourceModuleInstances.Init(nullptr, ModuleSlotCount);

		const int32 CopyCount = FMath::Min(ReplicatedSlotState.ModuleInstances.Num(), ModuleSlotCount);
		for (int32 Index = 0; Index < CopyCount; ++Index)
		{
			SlotState.SourceModuleInstances[Index] = ReplicatedSlotState.ModuleInstances[Index];
		}

		RebuildSkill(ReplicatedSlotState.SlotTag);
	}
}

void UMASkillManagerComponent::UpdateReplicatedSkillSlotRuntimeState(const FMASkillSlotRuntimeState& SlotState)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	FMASkillReplicatedSlotRuntimeState* ReplicatedSlotState = ReplicatedSkillSlotRuntimeStates.FindByPredicate(
		[&SlotState](const FMASkillReplicatedSlotRuntimeState& Candidate)
		{
			return Candidate.SlotTag == SlotState.SlotTag;
		});

	if (!ReplicatedSlotState)
	{
		ReplicatedSlotState = &ReplicatedSkillSlotRuntimeStates.AddDefaulted_GetRef();
		ReplicatedSlotState->SlotTag = SlotState.SlotTag;
	}

	const int32 ModuleSlotCount = GetModuleSlotCount(SlotState.SlotTag);
	ReplicatedSlotState->ModuleInstances.Init(nullptr, ModuleSlotCount);

	const int32 CopyCount = FMath::Min(SlotState.SourceModuleInstances.Num(), ModuleSlotCount);
	for (int32 Index = 0; Index < CopyCount; ++Index)
	{
		ReplicatedSlotState->ModuleInstances[Index] = SlotState.SourceModuleInstances[Index];
	}
}

void UMASkillManagerComponent::RefreshAbilityDefinition(FMASkillSlotRuntimeState& SlotState)
{
	UMASkillAbility* SkillAbility = ResolveSkillAbility(SlotState);
	if (!SkillAbility) return;

	SkillAbility->UpdateCurrentSkillModuleInstance(SlotState.AssembledModuleInstance);
}

UMASkillAbility* UMASkillManagerComponent::ResolveSkillAbility(const FMASkillSlotRuntimeState& SlotState) const
{
	if (!SlotState.AbilityHandle.IsValid()) return nullptr;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner());
	if (!AbilitySystemOwner) return nullptr;

	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent();
	if (!AbilitySystemComponent) return nullptr;

	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SlotState.AbilityHandle);
	if (!AbilitySpec) return nullptr;

	return Cast<UMASkillAbility>(AbilitySpec->GetPrimaryInstance());
}
