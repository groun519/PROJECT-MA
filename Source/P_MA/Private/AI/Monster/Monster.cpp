#include "AI/Monster/Monster.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "AI/Monster/MAMonsterCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAGameplayEffect_MonsterWaveStatScale.h"
#include "GAS/Skill/Addon/Sequence/MASkillModuleSequenceAddon.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Windup.h"

AMonster::AMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMAMonsterCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	SetGenericTeamId(FGenericTeamId(1));

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
}

void AMonster::BeginPlay()
{
	Super::BeginPlay();
	ApplyEnvMaterials();
}

void AMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMonster, EnvGameplayTag);
}

void AMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeSkills();
	ApplyStatCoefficientEffect();
}

void AMonster::SetEnvTag(const FGameplayTag& InEnvTag)
{
	if (EnvGameplayTag == InEnvTag) return;

	EnvGameplayTag = InEnvTag;
	ApplyEnvMaterials();

	if (HasAuthority()) ForceNetUpdate();
}

bool AMonster::IsActive() const
{
	return bActiveInPool && !IsDead();
}

void AMonster::Activate()
{
	GetWorldTimerManager().ClearTimer(DisappearTimerHandle);

	bActiveInPool = true;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	RespawnImmediately();

	if (AController* BaseCon = GetController())
	{
		if (AAIController* AICon = Cast<AAIController>(BaseCon))
		{
			if (UBrainComponent* Brain = AICon->GetBrainComponent())
			{
				Brain->StartLogic();
			}
		}
	}
}

void AMonster::Deactivate()
{
	ResetSkillSelection();

	if (HasAuthority())
	{
		if (UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponent()))
		{
			ASC->RemoveLooseGameplayTags(ASC->AppliedBaseTags);
			ASC->AppliedBaseTags.Reset();
		}
	}
	bActiveInPool = false;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	if (AController* BaseCon = GetController())
	{
		if (AAIController* AICon = Cast<AAIController>(BaseCon))
		{
			if (UBrainComponent* Brain = AICon->GetBrainComponent())
			{
				Brain->StopLogic(TEXT("Monster Deactivated"));
			}
		}
	}
}

void AMonster::ApplyStatCoefficientEffect()
{
	if (!HasAuthority()) return;
	if (FMath::IsNearlyEqual(StatCoefficient, 1.f)) return;

	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(GetAbilitySystemComponent());
	if (!ASC) return;

	UMAGameplayEffect_MonsterWaveStatScale::ApplyTo(*ASC, StatCoefficient);
	ASC->ApplyFullStatEffect();
}

void AMonster::ApplyEnvMaterials()
{
	USkeletalMeshComponent* MeshComp = GetMesh();

	const FMonsterEnvData* Found = nullptr;
	for (const FMonsterEnvData& Data : EnvTagToMaterial)
	{
		if (Data.EnvTag == EnvGameplayTag)
		{
			Found = &Data;
			break;
		}
	}
	if (!Found) return;

	const TArray<UMaterialInterface*>& MIList = Found->MIList;
	for (int32 Index = 0; Index < MIList.Num(); ++Index)
	{
		if (MIList[Index])
		{
			MeshComp->SetMaterial(Index, MIList[Index]);
		}
	}
}

void AMonster::InitializeSkills()
{
	UMASkillManagerComponent* SkillManager = GetSkillManagerComponent();
	if (!SkillManager) return;

	for (const FMonsterSkillSlotData& SkillSlot : SkillSlots)
	{
		if (!SkillSlot.SlotTag.IsValid()) continue;

		TArray<TObjectPtr<UMASkillModule>> Modules;
		LoadSkillModules(SkillSlot.Modules, Modules);
		SkillManager->ReplaceModulesAt(SkillSlot.SlotTag, Modules);
	}
}

bool AMonster::LoadSkillModules(
	const TArray<TSoftObjectPtr<UMASkillModule>>& ModuleAssets,
	TArray<TObjectPtr<UMASkillModule>>& OutModules)
{
	OutModules.Reset(ModuleAssets.Num());
	for (const TSoftObjectPtr<UMASkillModule>& ModuleAsset : ModuleAssets)
	{
		OutModules.Add(ModuleAsset.LoadSynchronous());
	}
	return OutModules.ContainsByPredicate([](const UMASkillModule* Module)
	{
		return Module != nullptr;
	});
}

bool AMonster::SelectWeightedSkill()
{
	SelectedSkillSlotTag = FGameplayTag();
	SelectedSkillUseDistance = 0.f;

	const UMASkillManagerComponent* SkillManager = GetSkillManagerComponent();
	if (!SkillManager) return false;

	float TotalWeight = 0.f;
	for (const FMonsterSkillSlotData& SkillSlot : SkillSlots)
	{
		if (SkillSlot.SelectionWeight <= 0.f || !SkillManager->GetAssembledModule(SkillSlot.SlotTag)) continue;
		TotalWeight += SkillSlot.SelectionWeight;
	}
	if (TotalWeight <= 0.f) return false;

	float RandomWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FMonsterSkillSlotData& SkillSlot : SkillSlots)
	{
		if (SkillSlot.SelectionWeight <= 0.f || !SkillManager->GetAssembledModule(SkillSlot.SlotTag)) continue;

		RandomWeight -= SkillSlot.SelectionWeight;
		if (RandomWeight > 0.f) continue;

		SelectedSkillSlotTag = SkillSlot.SlotTag;
		SelectedSkillUseDistance = SkillSlot.UseDistance;
		return true;
	}
	return false;
}

bool AMonster::SetPatternPlanFromStateNames(const TArray<FName>& StateNames)
{
	PatternPlan.Reset();
	if (!PatternDataTable) return false;

	for (const FName StateName : StateNames)
	{
		if (!ResolvePatternRow(PatternDataTable, StateName, TEXT("SetPatternPlanFromStateNames"))) continue;

		PatternPlan.Add(StateName);
	}
	return !PatternPlan.IsEmpty();
}

bool AMonster::SelectNextPatternPlanFragment()
{
	SelectedSkillSlotTag = FGameplayTag();
	SelectedSkillUseDistance = 0.f;
	if (!PatternSlotTag.IsValid() || PatternPlan.IsEmpty()) return false;

	const FName PatternRowName = PatternPlan[0];
	PatternPlan.RemoveAt(0);

	const FMonsterSkillPatternRow* PatternRow = ResolvePatternRow(
		PatternDataTable,
		PatternRowName,
		TEXT("SelectNextPatternPlanFragment"));
	if (!PatternRow || !ApplyPatternRowToActiveSlot(*PatternRow))
	{
		ResetSkillSelection();
		return false;
	}

	SelectedSkillSlotTag = PatternSlotTag;
	SelectedSkillUseDistance = PatternRow->UseDistance;
	return true;
}

void AMonster::ResetSkillSelection()
{
	SelectedSkillSlotTag = FGameplayTag();
	SelectedSkillUseDistance = 0.f;
	PatternPlan.Reset();
}

bool AMonster::ApplyPatternRowToActiveSlot(const FMonsterSkillPatternRow& PatternRow)
{
	UMASkillManagerComponent* SkillManager = GetSkillManagerComponent();
	if (!SkillManager) return false;

	TArray<TObjectPtr<UMASkillModule>> Modules;
	if (!LoadSkillModules(PatternRow.Modules, Modules)) return false;
	if (PatternRow.WindupDuration > 0.f)
	{
		// TODO: After submodule composition is available, express pattern windup as a
		// parameterized submodule instead of decorating a duplicated module here.
		const int32 TargetModuleIndex = Modules.IndexOfByPredicate([](const UMASkillModule* Module)
		{
			const UMASkillModuleSequenceAddon* SequenceAddon =
				Module ? Module->FindAddon<UMASkillModuleSequenceAddon>() : nullptr;
			return SequenceAddon && !SequenceAddon->GetSequences().IsEmpty();
		});
		if (TargetModuleIndex == INDEX_NONE) return false;

		UMASkillModule* TargetModule = DuplicateObject<UMASkillModule>(
			Modules[TargetModuleIndex],
			SkillManager);
		if (!TargetModule) return false;
		TargetModule->SetFlags(RF_Transient);

		UMASkillModuleSequenceAddon* SequenceAddon =
			TargetModule->FindMutableAddon<UMASkillModuleSequenceAddon>();
		if (!SequenceAddon) return false;

		UMASkillSequenceModifier_Windup* WindupModifier =
			SequenceAddon->AddTransientModifier<UMASkillSequenceModifier_Windup>();
		if (!WindupModifier) return false;

		WindupModifier->Configure(PatternRow.WindupDuration);
		Modules[TargetModuleIndex] = TargetModule;
	}

	return SkillManager->ReplaceModulesAt(PatternSlotTag, Modules)
		&& SkillManager->GetAssembledModule(PatternSlotTag);
}

const FMonsterSkillPatternRow* AMonster::ResolvePatternRow(const UDataTable* PatternDataTable, FName RowName, const TCHAR* Context)
{
	if (!PatternDataTable || RowName.IsNone()) return nullptr;
	return PatternDataTable->FindRow<FMonsterSkillPatternRow>(RowName, Context, false);
}

void AMonster::SetGoal(AActor* Goal)
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsObject(GoalBlackboardKeyName, Goal);
		}
	}
}

void AMonster::OnRep_EnvGameplayTag()
{
	ApplyEnvMaterials();
}

void AMonster::OnDead()
{
	Super::OnDead();
	ResetSkillSelection();
	OnMonsterDead.Broadcast();
	
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(DisappearTimerHandle);
		GetWorldTimerManager().SetTimer(
			DisappearTimerHandle,
			this, &AMonster::Deactivate,DisappearDelay,
			false
		);
	}
}
