// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/MAAbilityGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Setting/MASkillSubsystem.h"
#include "Widget/SkillDragDropOperation.h"
#include "Player/MAPlayerCharacter.h"
#include "Inventory/SkillBookComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputMappingContext.h"      
#include "Inventory/MAItemTypes.h" 
#include "InputAction.h" 

void UMAAbilityGauge::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (CooldownCounterText)
    {
       CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
    }

    OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

    WholeNumberFormattionOptions.MaximumFractionalDigits = 0;
    TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 1;

    if (Icon)
    {
       Icon->GetDynamicMaterial(); 
    }
}

void UMAAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    UMAAbilitySlotDataObject* DataItem = Cast<UMAAbilitySlotDataObject>(ListItemObject);
    if (DataItem)
    {
       this->AssignedInputID = DataItem->InputID;
       
       UpdateHotKeyText();
       
       UpdateSlot(DataItem->AbilityClass);
    }
}

void UMAAbilityGauge::UpdateHotKeyText()
{
    if (!HotKeyName) return;

    FString DisplayText = TEXT(""); 

    APlayerController* PC = GetOwningPlayer();
    if (PC && PC->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (UInputAction** FoundAction = InputActionMapping.Find(AssignedInputID))
            {
                if (UInputAction* Action = *FoundAction)
                {
                    // 현재 IMC에서 이 액션에 할당된 키 목록 가져오기
                    TArray<FKey> MappedKeys = Subsystem->QueryKeysMappedToAction(Action);
                    
                    if (MappedKeys.Num() > 0)
                    {
                        // 할당된 키가 있다면 짧은 이름으로 변환해서 텍스트로 지정
                        DisplayText = GetShortKeyName(MappedKeys[0]);
                    }
                }
            }
        }
    }

    HotKeyName->SetText(FText::FromString(DisplayText));
}

FString UMAAbilityGauge::GetShortKeyName(FKey Key) const
{
    if (Key == EKeys::RightMouseButton) return TEXT("MouseR");
    if (Key == EKeys::LeftMouseButton) return TEXT("MouseL");
    if (Key == EKeys::SpaceBar) return TEXT("Space");
    return Key.GetDisplayName().ToString();
}

void UMAAbilityGauge::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateMaxCooldownText();
}

bool UMAAbilityGauge::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
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
                SkillBook->EquipSkill(SkillOp->SkillClass, AssignedInputID);
                UpdateSlot(SkillOp->SkillClass);
                return true;
             }
          }
       }
    }
    
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UMAAbilityGauge::UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass)
{
    const FSkillItemData* WidgetData = FindWidgetDataForAbility(NewSkillClass);

    if (WidgetData && Icon)
    {
       UTexture2D* Texture = WidgetData->Icon.LoadSynchronous();
       if (Texture)
       {
          Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, Texture);
          Icon->SetVisibility(ESlateVisibility::Visible);
          Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
       }
    }
    else
    {
       // Icon->SetVisibility(ESlateVisibility::Hidden);
    }

    InitializeAbility(NewSkillClass);
}

const FSkillItemData* UMAAbilityGauge::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
    if (!AbilityDataTable) return nullptr;

    for (auto& RowPair : AbilityDataTable->GetRowMap())
    {
       const FSkillItemData* Data = reinterpret_cast<const FSkillItemData*>(RowPair.Value);
       if (Data && Data->GrantedAbility == AbilityClass)
       {
          return Data;
       }
    }
    return nullptr;
}

void UMAAbilityGauge::InitializeAbility(TSubclassOf<UGameplayAbility> NewAbilityClass)
{
    if (!NewAbilityClass)
    {
       AbilityCDO = nullptr;
       SharedCooldownTag = FGameplayTag();
       return;
    }

    AbilityCDO = NewAbilityClass->GetDefaultObject<UGameplayAbility>();
    if (UMAGameplayAbility_Skill* SkillCDO = Cast<UMAGameplayAbility_Skill>(AbilityCDO))
    {
       SharedCooldownTag = FGameplayTag::EmptyTag;

       if (UWorld* World = GetWorld())
       {
          if (UMASkillSubsystem* SkillSys = World->GetGameInstance()->GetSubsystem<UMASkillSubsystem>())
          {
             const FSkillData* SkillData = SkillSys->GetSkillData(SkillCDO->GetSkillID());
             if (SkillData && SkillData->CooldownTag.IsValid())
             {
                SharedCooldownTag = SkillData->CooldownTag;
             }
          }
       }
       if (SharedCooldownTag.IsValid() && OwnerASC.IsValid())
       {
          OwnerASC->RegisterGameplayTagEvent(SharedCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UMAAbilityGauge::OnCooldownTagChanged);
       }
    }

    UpdateMaxCooldownText();
    float Cost = UMAAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);
    if (CostText)  CostText->SetText(FText::AsNumber(Cost));
}

void UMAAbilityGauge::UpdateMaxCooldownText()
{
    if (!AbilityCDO || !OwnerASC.IsValid())
       return;

    float NewMaxCooldown = UMAAbilitySystemStatics::GetExpectedCooldownDuration(AbilityCDO,OwnerASC.Get());
    if (!FMath::IsNearlyEqual(CurrentDisplayMaxCooldown,NewMaxCooldown))
    {
       CurrentDisplayMaxCooldown = NewMaxCooldown;
       if (CooldownDurationText)
       {
          if (CurrentDisplayMaxCooldown>0.f)
          {
             CooldownDurationText->SetVisibility(ESlateVisibility::Visible);
             CooldownDurationText->SetText(FText::AsNumber(CurrentDisplayMaxCooldown));
          }
          else
          {
             CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
          }
       }
    }
}

void UMAAbilityGauge::OnCooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount)
{
    if (NewCount > 0)
    {
       if (!OwnerASC.IsValid()) return;
       
       FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(FGameplayTagContainer(SharedCooldownTag));

       TArray<TTuple<float,float>> Durations = OwnerASC->GetActiveEffectsTimeRemainingAndDuration(Query);
       if (Durations.Num() > 0)
       {
          const float CooldownTimeRemaining = Durations[0].Get<0>();
          const float CooldownDuration = Durations[0].Get<1>();
          StartCooldown(CooldownTimeRemaining, CooldownDuration);
       }
    }
    else
    {
       CooldownFinished();
    }
}

void UMAAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
    if(CooldownDurationText) CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
    
    CachedCooldownDuration = CooldownDuration;
    CachedCooldownTimeRemaining = CooldownTimeRemaining;

    if(CooldownCounterText) CooldownCounterText->SetVisibility(ESlateVisibility::Visible);

    GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UMAAbilityGauge::CooldownFinished, CachedCooldownTimeRemaining);
    GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle, this, &UMAAbilityGauge::UpdateCooldown, CooldownUpdateInterval, true, 0.f);
}

void UMAAbilityGauge::CooldownFinished()
{
    CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
    if(CooldownCounterText) CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
    GetWorld()->GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
    
    if(Icon)
    {
       Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
    }
}

void UMAAbilityGauge::UpdateCooldown()
{
    CachedCooldownTimeRemaining -= CooldownUpdateInterval;
    
    if (CachedCooldownTimeRemaining <= 0.f)
    {
       CooldownFinished();
       return;
    }

    FNumberFormattingOptions* FormattingOptions = CachedCooldownTimeRemaining > 1 ? &WholeNumberFormattionOptions : &TwoDigitNumberFormattingOptions;
    if(CooldownCounterText) CooldownCounterText->SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormattingOptions));

    if (Icon && CachedCooldownDuration > 0.f)
    {
       Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
    }
}