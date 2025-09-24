#include "GA_GolemChargeAttack.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UGA_GolemChargeAttack::UGA_GolemChargeAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_GolemChargeAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!K2_CommitAbility())
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[GolemCharge] Commit failed"));
        K2_EndAbility();
        return;
    }

    // 어빌리티 소유자 캐릭터
    ACharacter* OwnerChar = Cast<ACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr);
    AActor* Target = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    if (!OwnerChar || !Target)
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[GolemCharge] No Owner or Target"));
        K2_EndAbility();
        return;
    }

    // 거리 체크
    float Distance = FVector::Dist2D(OwnerChar->GetActorLocation(), Target->GetActorLocation());
    if (Distance < MinDistance)
    {
        if (bDebug) UE_LOG(LogTemp, Log, TEXT("[GolemCharge] Too close (%.1f < %.1f)"), Distance, MinDistance);
        K2_EndAbility();
        return;
    }

    // 차징 몽타주 실행
    if (ChargeMontage)
    {
        UAbilityTask_PlayMontageAndWait* Task =
            UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ChargeMontage);
        Task->OnCompleted.AddDynamic(this, &UGA_GolemChargeAttack::OnMontageFinished);
        Task->OnCancelled.AddDynamic(this, &UGA_GolemChargeAttack::OnMontageFinished);
        Task->OnInterrupted.AddDynamic(this, &UGA_GolemChargeAttack::OnMontageFinished);
        Task->OnBlendOut.AddDynamic(this, &UGA_GolemChargeAttack::OnMontageFinished);
        Task->ReadyForActivation();
    }
    else
    {
        if (bDebug) UE_LOG(LogTemp, Warning, TEXT("[GolemCharge] No ChargeMontage set"));
        K2_EndAbility();
    }
}

void UGA_GolemChargeAttack::OnMontageFinished()
{
    K2_EndAbility();
}
