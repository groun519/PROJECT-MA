// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/SkillBookComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

USkillBookComponent::USkillBookComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); 
}

void USkillBookComponent::BeginPlay()
{
	Super::BeginPlay();
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
    
	UE_LOG(LogTemp, Warning, TEXT("Skill Learned: %s"), *SkillClass->GetName());
}

void USkillBookComponent::EquipSkill(TSubclassOf<UGameplayAbility> SkillClass, EMAAbilityInputID SlotInputID)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (!ASC || !SkillClass) return;

	// 1. [중요] 해당 슬롯(Q, E, R...)에 이미 장착된 스킬이 있다면 제거
	if (EquippedSkills.Contains(SlotInputID))
	{
		FGameplayAbilitySpecHandle OldHandle = EquippedSkills[SlotInputID];
		if (OldHandle.IsValid())
		{
			ASC->ClearAbility(OldHandle); // 기존 스킬 제거!
		}
		EquippedSkills.Remove(SlotInputID);
	}

	// 2. 새 스킬 부여 (InputID를 지정해서!)
	// Spec 생성자의 3번째 인자가 InputID입니다. (int32로 캐스팅)
	FGameplayAbilitySpec Spec(SkillClass, 1, static_cast<int32>(SlotInputID), GetOwner());
    
	// 3. GAS에 등록
	FGameplayAbilitySpecHandle NewHandle = ASC->GiveAbility(Spec);

	// 4. 추적용 맵에 저장 (나중에 지울 때 쓰려고)
	if (NewHandle.IsValid())
	{
		EquippedSkills.Add(SlotInputID, NewHandle);
	}
	UE_LOG(LogTemp, Warning, TEXT("Equipped Skill [%s] to InputID [%d]"), *SkillClass->GetName(), (int32)SlotInputID);
}