#include "Inventory/SkillBookComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MAItemTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Modules/MASkillModuleData.h"

USkillBookComponent::USkillBookComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); 
}

void USkillBookComponent::BeginPlay()
{
	Super::BeginPlay();
	UAbilitySystemComponent* ASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC || !SkillDataTable) return;
	
	TArray<FSkillData*> AllSkillRows;
	SkillDataTable->GetAllRows<FSkillData>(TEXT("SkillPrewarming"), AllSkillRows);

	for (const FSkillData* RowData : AllSkillRows)
	{
		// 아이템 타입이 스킬이고, 실제 어빌리티 클래스가 세팅되어 있다면
		if (RowData && RowData->ItemType == EMAItemType::Skill && RowData->GrantedAbility)
		{
			TSubclassOf<UGameplayAbility> SkillClass = RowData->GrantedAbility;

			// 1. CDO 미리 불러오기 및 UI 연산 캐싱
			UGameplayAbility* CDO = SkillClass->GetDefaultObject<UGameplayAbility>();
			UMAAbilitySystemStatics::GetExpectedCooldownDuration(CDO, ASC);
			UMAAbilitySystemStatics::GetStaticCostForAbility(CDO);

			// 2. 인스턴싱 및 셰이더 컴파일 강제 실행 후 즉시 뺏기
			FGameplayAbilitySpec Spec(SkillClass, 1, INDEX_NONE, GetOwner());
			if (GetOwner() && GetOwner()->HasAuthority())
			{
				FGameplayAbilitySpecHandle TempHandle = ASC->GiveAbility(Spec);
				ASC->ClearAbility(TempHandle);
			}
		}
	}
}

bool USkillBookComponent::HasSkill(TSubclassOf<UGameplayAbility> SkillClass) const
{
	return LearnedSkills.Contains(SkillClass);
}

void USkillBookComponent::UnlockSkill(TSubclassOf<UGameplayAbility> SkillClass)
{
	Server_UnlockSkill(SkillClass);
}

void USkillBookComponent::Server_UnlockSkill_Implementation(TSubclassOf<UGameplayAbility> SkillClass)
{
	if (!SkillClass) return;
	if (HasSkill(SkillClass)) return; 
	
	LearnedSkills.Add(SkillClass);
	Client_UnlockSkill(SkillClass);
}

void USkillBookComponent::Client_UnlockSkill_Implementation(TSubclassOf<UGameplayAbility> SkillClass)
{
	if (!LearnedSkills.Contains(SkillClass))
	{
		LearnedSkills.Add(SkillClass);
	}
	
	OnSkillLearned.Broadcast(SkillClass);
    
	//UE_LOG(LogTemp, Warning, TEXT("Skill Learned: %s"), *SkillClass->GetName());
}

void USkillBookComponent::EquipSkill(TSubclassOf<UGameplayAbility> SkillClass, EMAAbilityInputID SlotInputID)
{
	//클라이언트에서 호출 시, 서버에게 장착하라고 부탁
	if (!GetOwner()->HasAuthority())
	{
		Server_EquipSkill(SkillClass, SlotInputID);
		return;
	}

	//서버에서 호출 시
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC || !SkillClass) return;
	
	if (EquippedSkills.Contains(SlotInputID))
	{
		FGameplayAbilitySpecHandle OldHandle = EquippedSkills[SlotInputID];
		if (OldHandle.IsValid())
		{
			ASC->ClearAbility(OldHandle);
		}
		EquippedSkills.Remove(SlotInputID);
	}
	
	FGameplayAbilitySpec Spec(SkillClass, 1, static_cast<int32>(SlotInputID), GetOwner());
	
	FGameplayAbilitySpecHandle NewHandle = ASC->GiveAbility(Spec);

	if (NewHandle.IsValid())
	{
		EquippedSkills.Add(SlotInputID, NewHandle);
	}
	//UE_LOG(LogTemp, Warning, TEXT("Equipped Skill [%s] to InputID [%d]"), *SkillClass->GetName(), (int32)SlotInputID);
}

void USkillBookComponent::Server_EquipSkill_Implementation(TSubclassOf<UGameplayAbility> SkillCalss,
	EMAAbilityInputID SlotInputID)
{
	EquipSkill(SkillCalss, SlotInputID);
}
