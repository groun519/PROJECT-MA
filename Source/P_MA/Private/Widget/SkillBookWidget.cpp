// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/SkillBookWidget.h"
#include "Widget/SkillSlotWidget.h"
#include "Inventory/SkillBookComponent.h"
#include "Components/WrapBox.h"
#include "Components/Button.h" 
#include "Player/MAPlayerCharacter.h" 
#include "GameFramework/PlayerController.h"

void USkillBookWidget::NativeConstruct()
{
    Super::NativeConstruct();

   if (SkillBookAnim)
   {
      PlayAnimation(SkillBookAnim);
   }

    if (CloseButton)
    {
       CloseButton->OnClicked.AddUniqueDynamic(this, &USkillBookWidget::OnCloseClicked);
    }

    if (!SkillList) return;
    SkillList->ClearChildren(); 
    
    if (APawn* OwnerPawn = GetOwningPlayerPawn())
    {
       if (AMAPlayerCharacter* MAChar = Cast<AMAPlayerCharacter>(OwnerPawn))
       {
          SkillBookComponent = MAChar->GetSkillBookComponent();
       }
    }

    if (SkillBookComponent)
    {
       for (const auto& SkillClass : SkillBookComponent->GetLearnedSkills())
       {
          AddSkillSlot(SkillClass);
       }
       
       SkillBookComponent->OnSkillLearned.AddUniqueDynamic(this, &USkillBookWidget::OnSkillLearned);
    }
}

void USkillBookWidget::OnCloseClicked()
{
    RemoveFromParent();
}

void USkillBookWidget::OnSkillLearned(TSubclassOf<UGameplayAbility> NewSkillClass)
{
    AddSkillSlot(NewSkillClass);
}

void USkillBookWidget::AddSkillSlot(TSubclassOf<UGameplayAbility> SkillClass)
{
    if (!SlotWidgetClass || !SkillList) return;
    
    USkillSlotWidget* NewSlot = CreateWidget<USkillSlotWidget>(this, SlotWidgetClass);
    if (NewSlot)
    {
       NewSlot->Init(SkillClass, EMAAbilityInputID::None); 
       
       SkillList->AddChildToWrapBox(NewSlot);
    }
}