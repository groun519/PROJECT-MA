#include "AI/BTTask_SendInputToAbilitySystem.h"
#include "AI/Golem/Monster.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GAS/MAAttributeSet.h"
#include "AIController.h"

DEFINE_LOG_CATEGORY_STATIC(LogBTTaskAbilitySystem, Log, All);

EBTNodeResult::Type UBTTask_SendInputToAbilitySystem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
		return EBTNodeResult::Failed;

	AMonster* Monster = Cast<AMonster>(AIC->GetPawn());
	if (!Monster)
		return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Monster);
	if (!ASC)
		return EBTNodeResult::Failed;

	const UMAAttributeSet* Attr = ASC->GetSet<UMAAttributeSet>();
	if (!Attr)
		return EBTNodeResult::Failed;

	const float Fury = Attr->GetFury();
	const float Threshold = Monster->FuryThreshold;
	const EMAAbilityInputID InputToUse = (Fury >= Threshold)
		? EMAAbilityInputID::Skill1
		: EMAAbilityInputID::Attack;

	const TCHAR* InputName = (InputToUse == EMAAbilityInputID::Skill1) ? TEXT("Skill1") : TEXT("Attack");

	ASC->PressInputID(static_cast<int32>(InputToUse));

	if (InputToUse == EMAAbilityInputID::Skill1)
	{
		FGameplayTag EndEventTag = FGameplayTag::RequestGameplayTag(TEXT("Monster.Ability.End"));
		FGameplayTagContainer TagContainer(EndEventTag);

		FDelegateHandle DelegateHandle = ASC->AddGameplayEventTagContainerDelegate(
			TagContainer,
			FGameplayEventTagMulticastDelegate::FDelegate::CreateLambda(
				[this, &OwnerComp, ASC, TagContainer, DelegateHandle](const FGameplayTag Tag, const FGameplayEventData* Payload) mutable
				{
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

					if (UBehaviorTreeComponent* BTC = &OwnerComp)
					{
						if (UWorld* World = BTC->GetWorld())
						{
							FTimerHandle RestartHandle;
							World->GetTimerManager().SetTimer(RestartHandle, [BTC]()
							{
								if (UBehaviorTree* BTAsset = BTC->GetCurrentTree())
								{
									BTC->RestartTree();
								}
							}, 0.1f, false);
						}
					}

					ASC->RemoveGameplayEventTagContainerDelegate(TagContainer, DelegateHandle);
				}
			)
		);

		return EBTNodeResult::InProgress;
	}
	return EBTNodeResult::Succeeded;
}
