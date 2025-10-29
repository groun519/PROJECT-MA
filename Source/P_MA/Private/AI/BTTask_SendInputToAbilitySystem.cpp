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
	UE_LOG(LogBTTaskAbilitySystem, Log, TEXT("[BTTask] Fury=%.1f / Threshold=%.1f → %s 입력"), Fury, Threshold, InputName);

	ASC->PressInputID(static_cast<int32>(InputToUse));

	if (InputToUse == EMAAbilityInputID::Skill1)
	{
		FGameplayTag EndEventTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Combo.Change.End"));
		FGameplayTagContainer TagContainer(EndEventTag);

		// ✅ Delegate 등록 및 Handle 저장
		FDelegateHandle DelegateHandle = ASC->AddGameplayEventTagContainerDelegate(
			TagContainer,
			FGameplayEventTagMulticastDelegate::FDelegate::CreateLambda(
				[this, &OwnerComp, ASC, TagContainer, DelegateHandle](const FGameplayTag Tag, const FGameplayEventData* Payload) mutable
				{
					UE_LOG(LogBTTaskAbilitySystem, Log, TEXT("[BTTask] Ability.Combo.Change.End 이벤트 수신 → FinishLatentTask + RestartTree"));

					// 1️⃣ Task 종료
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);

					// 2️⃣ Root 재시작 (0.1초 지연)
					if (UBehaviorTreeComponent* BTC = &OwnerComp)
					{
						if (UWorld* World = BTC->GetWorld())
						{
							FTimerHandle RestartHandle;
							World->GetTimerManager().SetTimer(RestartHandle, [BTC]()
							{
								if (UBehaviorTree* BTAsset = BTC->GetCurrentTree())
								{
									UE_LOG(LogBTTaskAbilitySystem, Log, TEXT("[BTTask] RestartTree() 호출"));
									BTC->RestartTree();
								}
							}, 0.1f, false);
						}
					}

					// 3️⃣ 델리게이트 해제 (태그 + 핸들 필요)
					ASC->RemoveGameplayEventTagContainerDelegate(TagContainer, DelegateHandle);
				}
			)
		);

		UE_LOG(LogBTTaskAbilitySystem, Log, TEXT("[BTTask] Skill1 InProgress → Ability.Combo.Change.End 이벤트 대기 중"));
		return EBTNodeResult::InProgress;
	}

	// Attack은 즉시 완료
	return EBTNodeResult::Succeeded;
}
