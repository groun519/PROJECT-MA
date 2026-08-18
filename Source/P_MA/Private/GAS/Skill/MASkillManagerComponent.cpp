#include "GAS/Skill/MASkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Animation/MAAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"
#include "GAS/Skill/Area/Decal/MASkillAreaDecalStatics.h"
#include "GAS/Skill/Definition/MASkillAssembler.h"
#include "GAS/Skill/Event/Dispatch/MASkillEventDispatcher.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/Event/Routing/MASkillEventRouter.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Runtime/MASkillRuntimeRegistry.h"
#include "GameFramework/Character.h"
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
	FGameplayTag VisualTag)
{
	SpawnSkillAreaImpactLocal(Area, VisualTag, false);
}

void UMASkillManagerComponent::Multicast_SpawnPredictedSkillAreaImpact_Implementation(
	FMASkillWorldAreaShape Area,
	FGameplayTag VisualTag)
{
	SpawnSkillAreaImpactLocal(Area, VisualTag, true);
}

void UMASkillManagerComponent::Multicast_RegisterSkillAreaPreviewContext_Implementation(
	UAnimSequenceBase* Animation,
	float ResolvedAreaScale,
	FGameplayTag VisualTag)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || OwnerActor->GetNetMode() == NM_DedicatedServer) return;
	if (OwnerActor->GetLocalRole() == ROLE_AutonomousProxy) return;

	const ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	USkeletalMeshComponent* Mesh = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	UMAAnimInstance* AnimInstance = Mesh ? Cast<UMAAnimInstance>(Mesh->GetAnimInstance()) : nullptr;
	if (!AnimInstance) return;

	AnimInstance->RegisterSkillAreaPreviewContext(Animation, ResolvedAreaScale, VisualTag);
}

void UMASkillManagerComponent::Multicast_UnregisterSkillAreaPreviewContext_Implementation(UAnimSequenceBase* Animation)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || OwnerActor->GetNetMode() == NM_DedicatedServer) return;
	if (OwnerActor->GetLocalRole() == ROLE_AutonomousProxy) return;

	const ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	USkeletalMeshComponent* Mesh = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	UMAAnimInstance* AnimInstance = Mesh ? Cast<UMAAnimInstance>(Mesh->GetAnimInstance()) : nullptr;
	if (!AnimInstance) return;

	AnimInstance->UnregisterSkillAreaPreviewContext(Animation);
}

void UMASkillManagerComponent::SpawnSkillAreaImpactLocal(
	const FMASkillWorldAreaShape& Area,
	FGameplayTag VisualTag,
	bool bSkipAutonomousProxy)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || OwnerActor->GetNetMode() == NM_DedicatedServer) return;
	if (bSkipAutonomousProxy && OwnerActor->GetLocalRole() == ROLE_AutonomousProxy) return;

	MASkillAreaDecalStatics::SpawnImpactLocal(OwnerActor, VisualTag, Area);
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

/** Module Lifetime **/
UMASkillModuleInstance* UMASkillManagerComponent::CreateModuleInstance(UMASkillModule* RootModule)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority() || !RootModule) return nullptr;

	UMASkillModuleInstance* ModuleInstance = NewObject<UMASkillModuleInstance>(OwnerActor);
	if (!ModuleInstance->SetRootModule(RootModule)) return nullptr;
	OwnerActor->AddReplicatedSubObject(ModuleInstance, COND_OwnerOnly);
	return ModuleInstance;
}

void UMASkillManagerComponent::UnregisterModuleInstance(UMASkillModuleInstance* ModuleInstance)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !ModuleInstance) return;

	ModuleInstance->OnSubModulesChanged.RemoveAll(this);
	OwnerActor->RemoveReplicatedSubObject(ModuleInstance);
}

/** Slot Composition **/
bool UMASkillManagerComponent::ReplaceModuleAt(
	FGameplayTag SlotTag,
	int32 ModuleIndex,
	UMASkillModule* NewModule)
{
	if (!CanMutateSkillSlots()) return false;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return false;
	if (!IsValidModuleSlotIndex(SlotTag, ModuleIndex)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotTag, SlotState.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstance = SlotState.SourceModuleInstances[ModuleIndex];
	UMASkillModuleInstance* NewModuleInstance = NewModule
		? CreateModuleInstance(NewModule)
		: nullptr;
	if (NewModule && !NewModuleInstance) return false;
	SlotState.SourceModuleInstances[ModuleIndex] = NewModuleInstance;

	if (PreviousModuleInstance != NewModuleInstance && PreviousModuleInstance)
	{
		PreviousModuleInstance->SetInSkillSlot(false);
	}
	RebuildSkill(SlotTag);
	UnregisterModuleInstance(PreviousModuleInstance);
	return true;
}

bool UMASkillManagerComponent::ReplaceModulesAt(
	FGameplayTag SlotTag,
	const TArray<TObjectPtr<UMASkillModule>>& NewModules)
{
	TArray<FMASkillModuleGroup> ModuleGroups;
	ModuleGroups.Reserve(NewModules.Num());
	for (UMASkillModule* Module : NewModules)
	{
		ModuleGroups.AddDefaulted_GetRef().RootModule = Module;
	}
	return ReplaceModulesAt(SlotTag, ModuleGroups);
}

bool UMASkillManagerComponent::ReplaceModulesAt(
	FGameplayTag SlotTag,
	const TArray<FMASkillModuleGroup>& NewModuleGroups)
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
		const FMASkillModuleGroup* ModuleGroup = NewModuleGroups.IsValidIndex(Index)
			? &NewModuleGroups[Index]
			: nullptr;
		if (!ModuleGroup || !ModuleGroup->RootModule) continue;

		UMASkillModuleInstance* ModuleInstance = CreateModuleInstance(ModuleGroup->RootModule);
		NewModuleInstances[Index] = ModuleInstance;
		if (ModuleInstance)
		{
			for (int32 SubModuleIndex = 0; SubModuleIndex < ModuleGroup->SubModules.Num(); ++SubModuleIndex)
			{
				UMASkillModule* SubModule = ModuleGroup->SubModules[SubModuleIndex];
				if (SubModule && !ModuleInstance->SetSubModuleAt(SubModuleIndex, SubModule))
				{
					ModuleInstance = nullptr;
					break;
				}
			}
		}
		if (ModuleInstance) continue;

		for (UMASkillModuleInstance* CreatedModuleInstance : NewModuleInstances)
		{
			UnregisterModuleInstance(CreatedModuleInstance);
		}
		return false;
	}
	SlotState.SourceModuleInstances = NewModuleInstances;

	for (UMASkillModuleInstance* PreviousModuleInstance : PreviousModuleInstances)
	{
		if (PreviousModuleInstance) PreviousModuleInstance->SetInSkillSlot(false);
	}
	RebuildSkill(SlotTag);
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

	if (PreviousModuleInstance != NewModuleInstance && PreviousModuleInstance)
	{
		PreviousModuleInstance->SetInSkillSlot(false);
	}
	RebuildSkill(SlotTag);
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

	Swap(SlotStateA.SourceModuleInstances[IndexA], SlotStateB.SourceModuleInstances[IndexB]);

	if (SlotTagA == SlotTagB)
	{
		RebuildSkill(SlotTagA);
		return true;
	}

	RebuildSkill(SlotTagA);
	RebuildSkill(SlotTagB);
	return true;
}

void UMASkillManagerComponent::ServerSwapModuleSlotsBetween_Implementation(
	FGameplayTag SlotTagA,
	int32 IndexA,
	FGameplayTag SlotTagB,
	int32 IndexB)
{
	SwapModuleSlotsBetween(SlotTagA, IndexA, SlotTagB, IndexB);
}

UMASkillModuleInstance* UMASkillManagerComponent::GetModuleInstanceAt(
	const FGameplayTag SlotTag,
	const int32 ModuleIndex)
{
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)
		|| !IsValidModuleSlotIndex(SlotTag, ModuleIndex))
	{
		return nullptr;
	}

	return FindOrAddSlotRuntimeState(SlotTag).SourceModuleInstances[ModuleIndex];
}

UMASkillModule* UMASkillManagerComponent::GetAssembledModule(FGameplayTag SlotTag) const
{
	const FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(SlotTag);
	const UMASkillModuleInstance* AssembledModuleInstance = SlotState ? SlotState->AssembledModuleInstance : nullptr;
	return AssembledModuleInstance ? AssembledModuleInstance->GetRootModule() : nullptr;
}

/** Slot Runtime **/
void UMASkillManagerComponent::RebuildSkill(FGameplayTag SlotTag)
{
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	for (UMASkillModuleInstance* ModuleInstance : SlotState.SourceModuleInstances)
	{
		if (!ModuleInstance) continue;

		ModuleInstance->OnSubModulesChanged.RemoveAll(this);
		ModuleInstance->OnSubModulesChanged.AddUObject(
			this,
			&UMASkillManagerComponent::HandleSubModulesChanged);
	}
	UMASkillModuleInstance* PreviousAssembledModuleInstance = SlotState.AssembledModuleInstance;
	if (UMASkillAbility* SkillAbility = ResolveSkillAbility(SlotState);
		SkillAbility && SkillAbility->IsActive())
	{
		SkillAbility->EndSkill();
	}

	SlotState.AssembledModuleInstance = FMASkillAssembler::Assemble(
		this,
		SlotTag,
		SlotState.SourceModuleInstances);
	if (FMASkillSystemStatics::IsActiveSkillSlotTag(SlotTag) || SlotState.AssembledModuleInstance)
	{
		EnsureAbilityForSlot(SlotState);
	}
	RefreshAbilityModule(SlotState);
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

	for (UMASkillModuleInstance* ModuleInstance : SlotState.SourceModuleInstances)
	{
		if (ModuleInstance) ModuleInstance->SetInSkillSlot(true);
	}

	NotifyActiveModulesChanged(SlotState);
	OnSkillSlotChanged.Broadcast(SlotTag);
}

void UMASkillManagerComponent::HandleSubModulesChanged(UMASkillModuleInstance* ModuleInstance)
{
	if (!ModuleInstance) return;

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		if (!SlotState.SourceModuleInstances.Contains(ModuleInstance)) continue;

		RebuildSkill(SlotState.SlotTag);
		return;
	}
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

	RefreshAbilityModule(SlotState);
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
	const UMASkillModule* SkillModule = SlotState.AssembledModuleInstance
		? SlotState.AssembledModuleInstance->GetRootModule()
		: nullptr;
	ActivePreviewVisualElementTag = SkillModule
		? SkillModule->GetVisualElementTag()
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

void UMASkillManagerComponent::NotifyActiveModulesChanged(const FMASkillSlotRuntimeState& SlotState)
{
	UMASkillAbility* SkillAbility = ResolveSkillAbility(SlotState);
	if (!SkillAbility || !SlotState.AssembledModuleInstance) return;

	const FGameplayTag EventTag = UMAAbilitySystemStatics::GetModuleActivationChangedEventTag();
	for (UMASkillModuleInstance* ModuleInstance : SlotState.SourceModuleInstances)
	{
		if (!ModuleInstance || !ModuleInstance->IsActive()) continue;

		bool bHasEventSource = false;
		MASkillModuleAddonStatics::ForEachAddon(
			*ModuleInstance,
			[&bHasEventSource, EventTag](const UMASkillModuleAddon& Addon)
			{
				const UMASkillModuleEventSourceAddon* EventSourceAddon =
					Cast<UMASkillModuleEventSourceAddon>(&Addon);
				bHasEventSource |= EventSourceAddon
					&& EventSourceAddon->HasEventSource(EventTag);
			});
		if (!bHasEventSource) continue;

		UMASkillEventRoutingStatics::TryNotifySkillEvent(
			SkillAbility,
			EventTag,
			FMASkillScopes(ModuleInstance, SlotState.AssembledModuleInstance));
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
	ModuleInstances.SetNum(GetModuleSlotCount(SlotTag));
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
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(SlotState.SlotTag)) continue;

		SlotState.AbilityHandle = AbilitySpec.Handle;
		return true;
	}

	const int32 SlotInputID = FMASkillSystemStatics::ResolveSlotInputID(SlotState.SlotTag);
	FGameplayAbilitySpec AbilitySpec(UMASkillAbility::StaticClass(), 1, SlotInputID, nullptr);
	AbilitySpec.GetDynamicSpecSourceTags().AddTag(SlotState.SlotTag);
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
		for (UMASkillModuleInstance* PreviousModuleInstance : SlotState.SourceModuleInstances)
		{
			if (!PreviousModuleInstance) continue;

			const bool bStillInSkillSlot = ReplicatedSkillSlotRuntimeStates.ContainsByPredicate(
				[PreviousModuleInstance](const FMASkillReplicatedSlotRuntimeState& Candidate)
				{
					return Candidate.ModuleInstances.Contains(PreviousModuleInstance);
				});
			if (!bStillInSkillSlot) PreviousModuleInstance->SetInSkillSlot(false);
		}

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

void UMASkillManagerComponent::RefreshAbilityModule(FMASkillSlotRuntimeState& SlotState)
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
