// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Passive/GAP_Jump.h"

#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"

UGAP_Jump::UGAP_Jump()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	// 1. 단순 이동 점프를 위한 트리거
	FAbilityTriggerData MovementTrigger;
	MovementTrigger.TriggerTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump");
	MovementTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(MovementTrigger);

	// 2. 공격 점프(내려찍기)를 위한 트리거
	FAbilityTriggerData DamageTrigger;
	DamageTrigger.TriggerTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Hit");
	DamageTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(DamageTrigger);
	
	ActivationBlockedTags.RemoveTag(UMAAbilitySystemStatics::GetStunStatTag());
}

void UGAP_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}
	// [핵심] 어떤 태그로 어빌리티가 활성화되었는지 확인하고, 그에 맞는 함수를 호출
	if (TriggerEventData->EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Hit")))
	{
		// ".Hit" 태그가 포함된 이벤트라면 피해 처리 함수 호출
		HandleDamage(TriggerEventData);
	}
	else if (TriggerEventData->EventTag.MatchesTag(FGameplayTag::RequestGameplayTag("Ability.Movement.Jump")))
	{
		// 일반 점프 이벤트라면 이동 처리 함수 호출
		HandleMovement(TriggerEventData);
	}

	// 작업이 끝났으므로 어빌리티 종료
	K2_EndAbility();
}

void UGAP_Jump::HandleMovement(const FGameplayEventData* TriggerEventData)
{
	// 캐릭터를 가져와서 위로 띄웁니다.
	ACharacter* MyCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (MyCharacter)
	{
		// Z축 속도만 적용하여 제자리에서 점프하도록 합니다.
		MyCharacter->LaunchCharacter(FVector(0.f, 0.f, JumpZVel), false, true);
	}
}

void UGAP_Jump::HandleDamage(const FGameplayEventData* TriggerEventData)
{
	if (!DamageGameplayEffect)
	{
		UE_LOG(LogTemp, Warning, TEXT("GAP_Jump: DamageGameplayEffect is not set!"));
		return;
	}

	// =================================================================================
	// ===== [수정된 부분] 존재하지 않는 함수 대신, 직접 TargetData를 생성합니다. =====
	// =================================================================================

	// 1. 최종적으로 TargetData를 담을 핸들을 생성합니다.
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	// 2. 위치 정보를 담을 수 있는 FGameplayAbilityTargetData_LocationInfo 타입의 데이터를 새로 만듭니다.
	FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();

	// 3. 데이터의 위치 타입을 'Transform'으로 지정하고, 현재 캐릭터의 Transform 정보를 넣어줍니다.
	//    이렇게 하면 이 데이터는 '캐릭터가 있는 위치'라는 정보를 가지게 됩니다.
	LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData->TargetLocation.LiteralTransform = GetAvatarActorFromActorInfo()->GetActorTransform();
	
	// 4. 생성한 위치 데이터를 핸들에 추가합니다.
	TargetDataHandle.Add(LocationData);

	// 이제 정상적으로 생성된 TargetDataHandle을 사용하여 주변 적들을 찾습니다.
	TArray<FHitResult> HitResults = GetHitResultFromVirtualSocketTargetData(TargetDataHandle, ETeamAttitude::Hostile, ShouldDrawDebug(), true);
	
	// 찾은 모든 적들에게 데미지 이펙트를 적용합니다.
	for (const FHitResult& HitResult : HitResults)
	{
		ApplyGameplayEffectToHitResultActor(HitResult, DamageGameplayEffect, GetAbilityLevel());
	}
}

