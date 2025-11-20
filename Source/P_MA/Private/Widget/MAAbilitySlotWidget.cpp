// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilitySlotWidget.h"
#include "Widget/SkillDragDropOperation.h"
#include "Components/Image.h"
#include "Player/MAPlayerCharacter.h"
#include "Inventory/SkillBookComponent.h"

void UMAAbilitySlotWidget::UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass)
{
	if (SkillIcon)
	{
		if (NewSkillClass)
		{
			// 실제 프로젝트에선 아이콘을 CDO나 데이터 테이블에서 가져와야 합니다.
			// 예시: UGameplayAbility* CDO = NewSkillClass->GetDefaultObject<UGameplayAbility>();
			// SkillIcon->SetBrushFromTexture(...);
			SkillIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SkillIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

bool UMAAbilitySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	USkillDragDropOperation* SkillOp = Cast<USkillDragDropOperation>(InOperation);
	if (SkillOp && SkillOp->SkillClass)
	{
		if (APawn* OwnerPawn = GetOwningPlayerPawn())
		{
			if (AMAPlayerCharacter* MAChar = Cast<AMAPlayerCharacter>(OwnerPawn))
			{
				if (USkillBookComponent* SkillBook = MAChar->GetSkillBookComponent())
				{
					// [핵심] 스킬북에게 장착 요청!
					SkillBook->EquipSkill(SkillOp->SkillClass, AssignedInputID);
                    
					// UI 갱신 (선택사항: 델리게이트로 처리해도 됨)
					UpdateSlot(SkillOp->SkillClass);
					return true;
				}
			}
		}
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}