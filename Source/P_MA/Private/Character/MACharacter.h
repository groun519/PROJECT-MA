// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "GAS/MAGameplayAbilityTypes.h" // 일단 문제가 있어서 이렇게 했는데 왜인지 모르겠음
#include "Abilities/GameplayAbility.h" // 일단 문제가 있어서 이렇게 했는데 왜인지 모르겠음
#include "MACharacter.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FMaterialParamData
{
	GENERATED_BODY()
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float Opacity = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Emissive = 0.f;
};

USTRUCT(BlueprintType)
struct FMaterialParamDataPair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMaterialParamData BodyData	= FMaterialParamData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMaterialParamData EyeData	= FMaterialParamData();
};

UCLASS()
class AMACharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AMACharacter();
	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledByPlayer() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	const TMap<EMAAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const; // 이거 문제때문에임
	
	virtual void PossessedBy(AController* NewController) override;
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Gameplay Ability **/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& EventTag, const FGameplayEventData& EventData);

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentGiantSwingInstigator;
	
private:
	void BindGASChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void StunTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void AimTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void ChargeTagUpdated(const FGameplayTag Tag, int32 NewCount);

	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	class UMAAbilitySystemComponent* MAAbilitySystemComponent;
	UPROPERTY()
	class UMAAttributeSet* MAAttributeSet;

	/** UI **/
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "UI")
	class UWidgetComponent* OverHeadWidgetComponent;

	void ConfigureOverHeadStatusWidget();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityCheckUpdateGap = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityRangeSquared = 1000000.f;

	FTimerHandle HeadStatGaugeVisibilityUpdateTimerHandle;

	void UpdateHeadGaugeVisibility();

	void SetStatusGaugeEnabled(bool bIsEnabled);

	/** Stun **/
private:
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	UAnimMontage* StunMontage;

	virtual void OnStun();
	virtual void OnRecoverFromStun();
	
	/** Death and Respawn **/
public:
	bool IsDead() const;
	void RespawnImmediately();
	
private:
	FTransform MeshRelativeTransform;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageFinishTimeShift = -0.8f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	FTimerHandle DeathMontageTimerHandle;

	// void DeathMontageFinished();
	// void SetRagdollEnabled(bool bIsEnabled);
	
	void PlayDeathAnimation();
	
	void StartDeathSequence();
	void Respawn();

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
	UMaterialInstanceDynamic* DynMat;
	
	UPROPERTY(ReplicatedUsing=OnRep_MaterialParam)
	FMaterialParamDataPair MaterialParamValue;
	UFUNCTION()
	void OnRep_MaterialParam();
	
	void ApplyMaterialParam();
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMaterialParamDataPair BaseMaterialParam;
	UFUNCTION()
	FORCEINLINE FMaterialParamDataPair& GetBaseMaterialParam() { return BaseMaterialParam; }
	
	// 서버에서 파라미터를 바꾸는 함수
	UFUNCTION(Server, Reliable)
	void Server_SetMaterialParams(const FMaterialParamData& BodyData, const FMaterialParamData& EyeData);

	/***************************************************************/
	/*								Skill						   */
	/***************************************************************/
public:
	//월드에 VFX 출력
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayNiagara(UNiagaraSystem* NS, FTransform SpawnTransform, bool bApplyColor=false, FLinearColor EffectColor=FLinearColor::White);
	//소켓에 VFX 부착
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayNiagaraAttached(UNiagaraSystem* NS, FName SocketName, FVector LocOffset, FRotator RotOffset, FVector Scale, bool bAutoDestroy, bool bApplyColor=false, FLinearColor EffectColor=FLinearColor::White);

	/** Airborne **/
public:
	virtual void Landed(const FHitResult& Hit) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Airborne")
	UAnimMontage* AirborneMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Airborne")
	UAnimMontage* KnockdownMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Airborne")
	UAnimMontage* RecoveryMontage;
	
	bool bIsAirborne = false;
};
