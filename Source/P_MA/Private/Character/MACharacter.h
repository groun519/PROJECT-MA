#pragma once

#include "CoreMinimal.h"
#include "Character/MAStatusEffectTypes.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "MACharacter.generated.h"

class UNiagaraSystem;
class UMAElementalComponent;
class UMAImpulseComponent;
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
	UMASkillManagerComponent* GetSkillManagerComponent() const { return SkillManagerComponent; }
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& EventTag, const FGameplayEventData& EventData);

private:
	void BindGASChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void MoveBlockTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void RefreshMaxWalkSpeed();
	void StopMovementForBlock();

	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UMAAbilitySystemComponent* MAAbilitySystemComponent;
	UPROPERTY()
	class UMAAttributeSet* MAAttributeSet;

	UPROPERTY(VisibleDefaultsOnly, Category = "Status Effect")
	UMAStatusEffectComponent* StatusEffectComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Elemental")
	TObjectPtr<UMAElementalComponent> ElementalComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Status Effect")
	TObjectPtr<UMAImpulseComponent> ImpulseComponent;

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

	/** Mat System **/
protected:
	UPROPERTY(VisibleDefaultsOnly, Category = "Loadout")
	class ULoadoutComponent* LoadoutComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Skill")
	TObjectPtr<UMASkillManagerComponent> SkillManagerComponent;

public:
	UFUNCTION(Server, Reliable)
	void Server_SetMaterialParams(const FMaterialParamData& BodyData, const FMaterialParamData& EyeData);

	/***************************************************************/
	/*								Skill						   */
	/***************************************************************/
public:
	//월드에 VFX 출력
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayNiagara(UNiagaraSystem* NS, FTransform SpawnTransform, bool bApplyColor=false, FLinearColor EffectColor=FLinearColor::White);
	UFUNCTION(NetMulticast, Reliable)
	// Kept on AMACharacter because the actor already owns the replication entrypoint.
	// If status-effect impulse RPCs grow, move this multicast into UMAStatusEffectComponent.
	void Multicast_PlayStatusEffectImpulse(const FGameplayTag& StatusEffectTag, float Magnitude, FVector SourcePoint);

	UFUNCTION()
	bool GetStatusEffectAnimConfig(const FGameplayTag& StatusEffectTag, FStatusEffectAnimConfig& OutConfig) const;
};
