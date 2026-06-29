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
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Step/MASkillStep_Cast.h"

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
		SkillManager->ReplaceDefinitionsAt(SkillSlot.SlotTag, SkillSlot.Definitions);
	}
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
		if (SkillSlot.SelectionWeight <= 0.f || !SkillManager->GetAssembledDefinition(SkillSlot.SlotTag)) continue;
		TotalWeight += SkillSlot.SelectionWeight;
	}
	if (TotalWeight <= 0.f) return false;

	float RandomWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FMonsterSkillSlotData& SkillSlot : SkillSlots)
	{
		if (SkillSlot.SelectionWeight <= 0.f || !SkillManager->GetAssembledDefinition(SkillSlot.SlotTag)) continue;

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
	if (!PatternRow.Definitions.ContainsByPredicate([](const TObjectPtr<UMASkillDefinition>& Definition)
	{
		return Definition != nullptr;
	}))
	{
		return false;
	}

	UMASkillManagerComponent* SkillManager = GetSkillManagerComponent();
	if (!SkillManager) return false;

	TArray<TObjectPtr<UMASkillDefinition>> Definitions = PatternRow.Definitions;
	if (PatternRow.WindupDuration > 0.f)
	{
		UMASkillDefinition* WindupDefinition = NewObject<UMASkillDefinition>(SkillManager, NAME_None, RF_Transient);
		if (!WindupDefinition) return false;

		UMASkillStep_Cast* WindupStep = WindupDefinition->CreateRuntimeSkillStep<UMASkillStep_Cast>();
		if (!WindupStep) return false;

		WindupStep->Configure(
			PatternRow.WindupDuration,
			EMASkillCastMontageMode::BlendInNextMontage,
			nullptr);
		Definitions.Insert(WindupDefinition, 0);
	}

	return SkillManager->ReplaceDefinitionsAt(PatternSlotTag, Definitions)
		&& SkillManager->GetAssembledDefinition(PatternSlotTag);
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
