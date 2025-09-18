// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAGameplayAbility.h"
#include "GAS/Passive/GAP_Launched.h"
#include "Animation/AnimNotify_SendTracePoint.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"

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
	ETeamAttitude::Type TargetTeam,
	ETraceObjectType TraceObjType,
	bool bDrawDebug, bool bIgnoreSelf)
{
	TArray<FHitResult> OutResults;
	TSet<AActor*> HitActors;

	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	
	for (const TSharedPtr<FGameplayAbilityTargetData> TargetData : TargetDataHandle.Data)
	{
		const FVector LocalStart = TargetData->GetOrigin().GetTranslation(); // Base(Local)
		const FVector LocalEnd   = TargetData->GetEndPoint();                // Tip (Local)

		FVector StartLoc = TargetData->GetOrigin().GetTranslation();
		FVector EndLoc = TargetData->GetEndPoint();

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<AActor*> ActorsToIgnore;
		if (bIgnoreSelf)
		{
			ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
		}
		
		EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		TArray<FHitResult> Results;

		if (TraceObjType == ETraceObjectType::None)
		{
			return OutResults;
		}
		if (TraceObjType == ETraceObjectType::Box)
		{
			FRotator BoxWorldRot = GetAvatarActorFromActorInfo()->GetActorRotation() + BoxRot;
			UKismetSystemLibrary::BoxTraceMultiForObjects(
				this, StartLoc, EndLoc, HalfSize, BoxWorldRot,
				ObjectTypes, false, ActorsToIgnore, DrawDebugTrace, Results, false
				);
		}
		else if (TraceObjType == ETraceObjectType::Sphere)
		{
			UKismetSystemLibrary::SphereTraceMultiForObjects(
				this, StartLoc, EndLoc, HalfSize.X,
				ObjectTypes, false, ActorsToIgnore, DrawDebugTrace, Results, false
				);
		}

		else if (TraceObjType == ETraceObjectType::Line)
		{
			// 첫 이벤트면 저장만
			if (!bHasPrevSegment)
			{
				PrevBaseLocal  = LocalStart;
				PrevTipLocal   = LocalEnd;
				bHasPrevSegment = true;
			}
			else
			{
				// 생성할 때만 액터 위치 더해서(월드) 생성
				const AActor* Avatar   = GetAvatarActorFromActorInfo();
				const FTransform Basis = Avatar ? Avatar->GetActorTransform() : FTransform::Identity;

				const FVector PrevBaseW = Basis.TransformPosition(PrevBaseLocal);
				const FVector PrevTipW  = Basis.TransformPosition(PrevTipLocal);
				const FVector CurBaseW  = Basis.TransformPosition(LocalStart);
				const FVector CurTipW   = Basis.TransformPosition(LocalEnd);

				auto DoLine = [&](const FVector& S, const FVector& E)
				{
					TArray<FHitResult> Temp;
					UKismetSystemLibrary::LineTraceMultiForObjects(
						this, S, E,
						ObjectTypes, false, ActorsToIgnore,
						DrawDebugTrace, Temp, false,
						FLinearColor::Green, FLinearColor::Red, 2.0f
					);
					for(FHitResult result : Temp)
					{
						AActor* HitActor = result.GetActor();
						ActorsToIgnore.Add(HitActor);
					}
					Results.Append(Temp);
				};

				// 누락 방지용 4개 선분
				DoLine(PrevBaseW, CurBaseW);
				DoLine(PrevTipW,  CurTipW);
				DoLine(PrevBaseW, CurTipW);
				DoLine(PrevTipW,  CurBaseW);

				// 현재 로컬을 이전으로 갱신
				PrevBaseLocal = LocalStart;
				PrevTipLocal  = LocalEnd;
			}
		}
		
		for (const FHitResult& Result : Results)
		{
			// 스스로는 피해 x
			if (HitActors.Contains(Result.GetActor()))
			{
				continue;
			}

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
	}
	return OutResults;
}

TArray<FHitResult> UMAGameplayAbility::GetHitResultFromVirtualSocketTargetData(
	const FGameplayAbilityTargetDataHandle& Handle,
	ETeamAttitude::Type TargetTeam,
	bool bDrawDebug, bool bIgnoreSelf)
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
	if (!VS || !Loc) return {}; // 확실치 않음: 둘 다 필요. 한쪽 없으면 빈 배열 반환.

	FGameplayAbilityTargetDataHandle LocHandle;
	{
		auto* Copy = new FGameplayAbilityTargetData_LocationInfo(*Loc);
		LocHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(Copy));
	}
	
	// 2) 기존 범용 함수로 위임
	if (VS->Shape == EVA_Shape::Sphere)
	{
		return GetHitResultFromSweepLocationTargetData(
			LocHandle, FVector(VS->SphereRadius,0,0), VS->LocalRotation, TargetTeam, ETraceObjectType::Sphere, bDrawDebug, bIgnoreSelf);
	}
	else // Box
	{
		// 주의: 현재 박스는 ZeroRotator로 트레이스함. 박스 회전이 필요하면 아래 “선택 개선” 참고.
		return GetHitResultFromSweepLocationTargetData(
			LocHandle, VS->BoxHalfSize, VS->LocalRotation, TargetTeam, ETraceObjectType::Box, bDrawDebug, bIgnoreSelf);
	}
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
	if (!Target)
		return;

	FGameplayEventData EventData;		//GAS 이벤트 전달 컨테이너
	FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit;	//TargetData의 종류(FHitResult포함) -> 히트 기반 타깃 정보 담아
	FHitResult HitResult;				//충돌 결과 구조체 (히트 위치/노말/히트된 컴포넌트,액터 포함)
	HitResult.ImpactNormal = PushVel;
	HitData -> HitResult = HitResult;
	EventData.TargetData.Add(HitData);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, UGAP_Launched::GetLaunchedAbilityActivationTag(), EventData);
}

ACharacter* UMAGameplayAbility::GetOwningAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	return AvatarCharacter;
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
