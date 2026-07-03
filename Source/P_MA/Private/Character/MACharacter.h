#pragma once

#include "CoreMinimal.h"
#include "Character/MAStatusEffectTypes.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MACharacter.generated.h"

class UNiagaraSystem;
class UMAElementalComponent;
class UMAImpulseComponent;
class UMAOverlayComponent;
class UMAStatusEffectComponent;
class UMASkillManagerComponent;

UCLASS()
class AMACharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AMACharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledByPlayer() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PossessedBy(AController* NewController) override;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Gameplay Ability **/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;
	UMAImpulseComponent* GetImpulseComponent() const { return ImpulseComponent; }
	UMAStatusEffectComponent* GetStatusEffectComponent() const { return StatusEffectComponent; }
	UMAElementalComponent* GetElementalComponent() const { return ElementalComponent; }
	UMAOverlayComponent* GetOverlayComponent() const { return OverlayComponent; }
	UMASkillManagerComponent* GetSkillManagerComponent() const { return SkillManagerComponent; }
	
private:
	void BindGASChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void MoveBlockTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void RefreshMaxWalkSpeed();
	void StopMovementForBlock();

	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UMAAbilitySystemComponent* MAAbilitySystemComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	TObjectPtr<class UMAAttributeSet> MAAttributeSet;

	UPROPERTY(VisibleDefaultsOnly, Category = "Status Effect")
	UMAStatusEffectComponent* StatusEffectComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Elemental")
	TObjectPtr<UMAElementalComponent> ElementalComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Status Effect")
	TObjectPtr<UMAImpulseComponent> ImpulseComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Overlay")
	TObjectPtr<UMAOverlayComponent> OverlayComponent;

	/** UI **/
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	class UWidgetComponent* OverHeadWidgetComponent;

	void ConfigureOverHeadStatusWidget();
	class UMAOverHeadStatsGauge* EnsureOverHeadStatusWidgetConfigured();

	void SetStatusGaugeEnabled(bool bIsEnabled);
	bool bStatusGaugeEnabled = true;

	/** Death and Respawn **/
public:
	bool IsMovementBlocked() const;
	bool IsDead() const;
	void RespawnImmediately();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	void StartDeathSequence();
	void Respawn();

protected:
	virtual void OnDead();
	virtual void OnRespawn();

	/** Team **/
public:
	// Assigns Team Agent to given TeamID
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	// Retrieve team identifier in form of FGenericTeamId
	virtual FGenericTeamId GetGenericTeamId() const override;
private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID;

	UFUNCTION()
	virtual void OnRep_TeamID();
	
	/** AI **/
private:
	void SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled);
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;

	/** Skill **/
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Skill")
	TObjectPtr<UMASkillManagerComponent> SkillManagerComponent;

	/***************************************************************/
	/*								Skill						   */
	/***************************************************************/
public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AttachNiagaraToSelf(UNiagaraSystem* NS, FName SocketName, float LifeSpan);
	UFUNCTION()
	bool GetStatusEffectAnimConfig(const FGameplayTag& StatusEffectTag, FStatusEffectAnimConfig& OutConfig) const;
};
