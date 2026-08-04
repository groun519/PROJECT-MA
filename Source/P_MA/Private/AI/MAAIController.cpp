#include "AI/MAAIController.h"

#include "Character/MACharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AMAAIController::AMAAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = 1200.f;
	
	SightConfig->SetMaxAge(5.f);

	SightConfig->PeripheralVisionAngleDegrees = 180.f;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AMAAIController::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &AMAAIController::TargetForgotten);
}

void AMAAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(NewPawn);
	if (PawnTeamInterface)
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());
		ClearAndDisableAllSenses();
		EnableAllSenses();
	}

	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewPawn);
	if (PawnASC)
	{
		PawnASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &AMAAIController::PawnDeadTagUpdated);
		PawnASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetAnyReactionStateTag()).AddUObject(this, &AMAAIController::PawnReactionTagUpdated);
	}

	if (TargetRefreshInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TargetRefreshTimerHandle, this, &AMAAIController::RefreshCurrentTarget, TargetRefreshInterval, true);
	}
}

void AMAAIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);
}

void AMAAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TargetRefreshTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AMAAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		if (!GetCurrentTarget())
		{
			SetCurrentTarget(TargetActor);
		}
	}
	else
	{
		ForgetActorIfDead(TargetActor);
	}
}

void AMAAIController::TargetForgotten(AActor* ForgottenActor)
{
	if (!ForgottenActor) return;
	if (GetCurrentTarget() != ForgottenActor) return;

	AActor* NextTarget = GetNextPerceivedActor();
	if (NextTarget)
	{
		SetCurrentTarget(NextTarget);
		return;
	}

	const UAbilitySystemComponent* ForgottenActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ForgottenActor);
	const bool bForgottenActorDead = ForgottenActorASC && ForgottenActorASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetDeadStatTag());
	if (bForgottenActorDead)
	{
		SetCurrentTarget(nullptr);
	}
}

AActor* AMAAIController::GetNextPerceivedActor() const
{
	if (AIPerceptionComponent)
	{
		TArray<AActor*> Actors;
		AIPerceptionComponent->GetPerceivedHostileActors(Actors);

		APawn* ControlledPawn = GetPawn();
		AActor* BestActor = nullptr;
		float BestDistanceSq = TNumericLimits<float>::Max();
		for (AActor* Actor : Actors)
		{
			if (!Actor) continue;

			const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
			if (ActorASC && ActorASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetDeadStatTag())) continue;

			const float DistanceSq = ControlledPawn
				? FVector::DistSquared(ControlledPawn->GetActorLocation(), Actor->GetActorLocation())
				: 0.f;
			if (DistanceSq >= BestDistanceSq) continue;

			BestActor = Actor;
			BestDistanceSq = DistanceSq;
		}
		return BestActor;
	}
	return nullptr;
}

void AMAAIController::RefreshCurrentTarget()
{
	if (bIsPawnDead || bIsPawnReacting) return;

	AActor* NextTarget = GetNextPerceivedActor();
	if (NextTarget && NextTarget != GetCurrentTarget())
	{
		SetCurrentTarget(NextTarget);
	}
}

void AMAAIController::ForgetActorIfDead(AActor* ActorToForget)
{
	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC)
		return;

	if (ActorASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetDeadStatTag()))
	{
		for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter = AIPerceptionComponent->GetPerceptualDataIterator(); Iter; ++Iter)
		{
			if (Iter->Key != ActorToForget) continue;

			for (FAIStimulus& Stimuli : Iter->Value.LastSensedStimuli)
			{
				Stimuli.SetStimulusAge(TNumericLimits<float>::Max());
			}
		}
	}
}

AActor* AMAAIController::GetCurrentTarget() const
{
	const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent)
	{
		return Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(TargetBlackboardKeyName));
	}
	return nullptr;
}

void AMAAIController::SetCurrentTarget(AActor* NewTarget)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent) return;

	if (NewTarget)
	{
		BlackboardComponent->SetValueAsObject(TargetBlackboardKeyName, NewTarget);
	}
	else
	{
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

void AMAAIController::ClearAndDisableAllSenses()
{
	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());

	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
	}

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(TargetBlackboardKeyName);
	}
}

void AMAAIController::EnableAllSenses()
{
	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}

void AMAAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	bIsPawnDead = Count!=0;
	if (bIsPawnDead)
	{
		GetBrainComponent()->StopLogic("Dead");
		ClearAndDisableAllSenses();
		bIsPawnDead = true;
	}
	else
	{
		GetBrainComponent()->StartLogic();
		EnableAllSenses();
		bIsPawnDead = false;
	}
}

void AMAAIController::PawnReactionTagUpdated(const FGameplayTag /*Tag*/, int32 /*Count*/)
{
	if (bIsPawnDead) return;

	FGameplayTagContainer OwnedTags;
	if (UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
	{
		PawnASC->GetOwnedGameplayTags(OwnedTags);
	}

	OwnedTags.RemoveTag(UMAAbilitySystemStatics::GetKnockbackStatTag());
	const bool bShouldReact = OwnedTags.HasTag(UMAAbilitySystemStatics::GetAnyReactionStateTag());

	if (bShouldReact)
	{
		if (!bIsPawnReacting)
		{
			GetBrainComponent()->StopLogic("Reaction");
			bIsPawnReacting = true;
		}
	}else
	{
		if (bIsPawnReacting)
		{
			GetBrainComponent()->StartLogic();
			bIsPawnReacting = false;
		}
	}
}
