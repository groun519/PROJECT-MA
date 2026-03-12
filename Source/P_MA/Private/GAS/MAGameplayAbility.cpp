// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAGameplayAbility.h"
#include "Animation/AnimNotify_SendTracePoint.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

#include "DebugShapeHelper.h"
#include "MAAbilitySystemStatics.h"
#include "VirtualSocketTargetData.h"
#include "Engine/OverlapResult.h"
#include "P_MA/P_MA.h"


UMAGameplayAbility::UMAGameplayAbility()
{
	ActivationBlockedTags.AddTag(UMAAbilitySystemStatics::GetStunStatTag());
	BlockAbilitiesWithTag.AddTag(UMAAbilitySystemStatics::GetBasicAttackAbilityTag());
}

class UAnimInstance* UMAGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
	{
		return OwnerSkeletalMeshComp->GetAnimInstance();
	}
	return nullptr;
}

/** GetHitResultFromSweepLocationTargetData
 * @param TargetTeam 타겟의 팀을 받아오는게 아니라, 타게팅할 팀을 받아오는 매개변수
 */
TArray<FHitResult> UMAGameplayAbility::GetHitResultFromSweepLocationTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FVector HalfSize, FRotator BoxRot,
	bool bUseSector, float SectorAngle,
	ETeamAttitude::Type TargetTeam,
	EVA_Shape TraceObjType,
	bool bDrawDebug, bool bIgnoreSelf)
{
	TArray<FOverlapResult> OutResults;
	TSet<AActor*> HitActors;

	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());

	const TSharedPtr<FGameplayAbilityTargetData>& TargetData = TargetDataHandle.Data[0];

	FVector Center = TargetData->GetOrigin().GetTranslation();

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Hitbox);

	FCollisionQueryParams QueryParams;
	if (bIgnoreSelf) QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());

	TArray<FOverlapResult> OverlapResults;
	if (TraceObjType == EVA_Shape::None)
	{
		return {};
	}
	else if (TraceObjType == EVA_Shape::Sphere)
	{
		if (!bUseSector) // 원
		{
			GetWorld()->OverlapMultiByObjectType(
				OverlapResults, Center, FQuat::Identity, ObjParams,
				FCollisionShape::MakeSphere(HalfSize.X), QueryParams);

			if (bDrawDebug)
				FDebugShapeHelper::DrawDebugSectorableCircle(GetWorld(), Center, HalfSize.X, 32,
					false, 0.f, GetAvatarActorFromActorInfo()->GetActorForwardVector(),
					FColor::White, 1.f);
		}
		else // 부채꼴
		{
			GetWorld()->OverlapMultiByObjectType(
				OverlapResults, Center, FQuat::Identity, ObjParams,
				FCollisionShape::MakeSphere(HalfSize.X), QueryParams);

			if (bDrawDebug)
			{
				FDebugShapeHelper::DrawDebugSectorableCircle(GetWorld(), Center, HalfSize.X, 360,
					true, SectorAngle/2, GetAvatarActorFromActorInfo()->GetActorRotation().Vector(),
					FColor::White, 1.f);
			}
		}
	}
	else if (TraceObjType == EVA_Shape::Box) // 사각형
	{
		FQuat WorldQuat = GetAvatarActorFromActorInfo()->GetActorRotation().Quaternion();
		FQuat LocalQuat = BoxRot.Quaternion();
		FQuat FinalQuat = WorldQuat * LocalQuat;
		
		GetWorld()->OverlapMultiByObjectType(
			OverlapResults, Center, FinalQuat, ObjParams,
			FCollisionShape::MakeBox(HalfSize), QueryParams);

		if (bDrawDebug)
		{
			FDebugShapeHelper::DrawDebugRect(GetWorld(), Center, HalfSize.X, HalfSize.Y,
				FinalQuat.Vector(), FColor::White, 1.f);
		}
	}
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		// 중복 피격 방지
		if (HitActors.Contains(Result.GetActor())) continue;

		/** 대상과의 팀 관계(OtherActorTeamAttitude, PlayerTeam -> TargetTeam)가 TargetTeam과 같지 않으면 피해x
		 * ex1.	TargetTeam				= ETeamAttitude::Friendly	"이 스킬은 아군에게만 적용."
		 *		OtherActorTeamAttitude	= ETeamAttitude::Friendly	"대상 액터의 팀과의 관계는 아군."
		 *			=> 피격 가능 (힐 스킬인데, 적군을 캐스팅했음.)
		 * ex2.	OtherActorTeamAttitude  = ETeamAttitude::Hostile	"이 스킬은 적군에게만 적용."
		 *		TargetTeam				= ETeamAttitude::Friendly	"대상 액터의 팀과의 관계는 아군."
		 *			=> 피격 불가 (딜 스킬인데, 아군이 피격 범위에 존재했음.)
		 */
		if (OwnerTeamInterface)
		{
			ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*Result.GetActor());
			if (OtherActorTeamAttitude != TargetTeam)
			{
				continue;
			}
		}

		HitActors.Add(Result.GetActor());
		OutResults.Add(Result);
	}
	TArray<FHitResult> OutHits;
	FDebugShapeHelper::ConvertOverlapsToHitResults(OutResults, OutHits);
	
	return OutHits;
}

TArray<FHitResult> UMAGameplayAbility::GetHitResultFromVirtualSocketTargetData(
	const FGameplayAbilityTargetDataHandle& Handle)
{
	// 1) VS 데이터/위치 추출
	const FGameplayAbilityTargetData_VirtualSocket* VS = nullptr;
	const FGameplayAbilityTargetData_LocationInfo*  Loc= nullptr;
	
	for (const TSharedPtr<FGameplayAbilityTargetData>& TD : Handle.Data)
	{
		if (!VS && TD->GetScriptStruct() == FGameplayAbilityTargetData_VirtualSocket::StaticStruct())
			VS = static_cast<const FGameplayAbilityTargetData_VirtualSocket*>(TD.Get());
		if (!Loc && TD->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct())
			Loc = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(TD.Get());
	}
	if (!VS || !Loc) return {};

	FGameplayAbilityTargetDataHandle LocHandle;
	{
		auto* Copy = new FGameplayAbilityTargetData_LocationInfo(*Loc);
		LocHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(Copy));
	}
	
	// 2) 기존 범용 함수로 위임
	TArray<FHitResult> OutHits;
	if (VS->Shape == EVA_Shape::Sphere)
	{
		OutHits = GetHitResultFromSweepLocationTargetData(
			LocHandle, FVector(VS->SphereRadius,0,0),
			VS->LocalRotation, VS->bUseSector, VS->SectorAngle,
			VS->TargetTeam, EVA_Shape::Sphere, VS->bDrawDebug, VS->bIgnoreOwner);
	}
	else // Box
	{
		// 주의: 현재 박스는 ZeroRotator로 트레이스함. 박스 회전이 필요하면 아래 “선택 개선” 참고.
		OutHits = GetHitResultFromSweepLocationTargetData(
			LocHandle, VS->BoxHalfSize,
			VS->LocalRotation, false, 0,
			VS->TargetTeam, EVA_Shape::Box, VS->bDrawDebug, VS->bIgnoreOwner);
	}

	// 3) GameplayCue 실행
	for (FHitResult& Result : OutHits)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor) continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!TargetASC) continue;

		FGameplayCueParameters CueParam;
		CueParam.Location = Result.ImpactPoint;
		CueParam.Normal = Result.ImpactNormal;
		CueParam.Instigator = GetAvatarActorFromActorInfo();
		CueParam.EffectCauser = GetAvatarActorFromActorInfo();
		for (const FGameplayTag& GameplayCueTag : VS->TriggerGameplayCueTags)
		{
			TargetASC->ExecuteGameplayCue(GameplayCueTag, CueParam);
		}
	}

	return OutHits;
}

void UMAGameplayAbility::PushSelf(const FVector& PushVel)
{
	if (ACharacter* OwningAvatarCharacter = GetOwningAvatarCharacter())
	{
		OwningAvatarCharacter -> LaunchCharacter(PushVel, true, true);
	}
}

//대상 (Target 액터)에게 "발사/밀어내기" 이벤트 보내는 함수
void UMAGameplayAbility::PushTarget(AActor* Target, const FVector& PushVel)
{
	if (!Target)	return;

	FGameplayEventData EventData;
	FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit;
	FHitResult HitResult;
	HitResult.ImpactNormal = PushVel;
	HitData -> HitResult = HitResult;
	EventData.TargetData.Add(HitData);
	EventData.EventTag = UMAAbilitySystemStatics::GetLaunchActivateTag();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, EventData.EventTag, EventData);
}

void UMAGameplayAbility::StopMontageAfterCurrentSection(UAnimMontage* Montage)
{
	UAnimInstance* OwnerAnimInst = GetOwnerAnimInstance();
	if (OwnerAnimInst)
	{
		FName CurrentSectionName = OwnerAnimInst->Montage_GetCurrentSection(Montage);
		OwnerAnimInst->Montage_SetNextSection(CurrentSectionName, NAME_None, Montage);
	}
}

void UMAGameplayAbility::PlayMontageLocally(UAnimMontage* Montage)
{
	UAnimInstance* OwnerAnimIst = GetOwnerAnimInstance();
	if (OwnerAnimIst && !OwnerAnimIst->Montage_IsPlaying(Montage))
	{
		OwnerAnimIst->Montage_Play(Montage);
	}
}

ACharacter* UMAGameplayAbility::GetOwningAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	return AvatarCharacter;
}

FGenericTeamId UMAGameplayAbility::GetOwnerTeamId() const
{
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	if (OwnerTeamInterface)
	{
		return OwnerTeamInterface->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

void UMAGameplayAbility::ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult,
                                                             TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);
		
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	EffectContext.AddHitResult(HitResult);

	EffectSpecHandle.Data->SetContext(EffectContext);

	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
}
