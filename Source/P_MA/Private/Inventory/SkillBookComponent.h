#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "SkillBookComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillLearnedDelegate, TSubclassOf<UGameplayAbility>, SkillClass);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API USkillBookComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	USkillBookComponent();

	FOnSkillLearnedDelegate OnSkillLearned;
	
	bool HasSkill(TSubclassOf<UGameplayAbility> SkillClass) const;
	
	void UnlockSkill(TSubclassOf<UGameplayAbility> SkillClass);
	
	const TArray<TSubclassOf<UGameplayAbility>>& GetLearnedSkills() const { return LearnedSkills; }

	void EquipSkill(TSubclassOf<UGameplayAbility> SkillClass, EMAAbilityInputID SlotInputID);

	UFUNCTION(Server, Reliable)
	void Server_EquipSkill(TSubclassOf<UGameplayAbility> SkillCalss, EMAAbilityInputID SlotInputID);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* SkillDataTable;
	
	UPROPERTY()
	TArray<TSubclassOf<UGameplayAbility>> LearnedSkills;

	UFUNCTION(Server, Reliable)
	void Server_UnlockSkill(TSubclassOf<UGameplayAbility> SkillClass);
	
	UFUNCTION(Client, Reliable)
	void Client_UnlockSkill(TSubclassOf<UGameplayAbility> SkillClass);
	
private:
	TMap<EMAAbilityInputID, FGameplayAbilitySpecHandle> EquippedSkills;
};
