// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAGameplayAbility.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "DebugShapeHelper.h"
#include "VirtualSocketTargetData.h"
#include "Engine/OverlapResult.h"

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

	const TSharedPtr<FGameplayAbilityTargetData> TargetData;

	FVector Start = TargetData->GetOrigin().GetTranslation();
	FVector End = TargetData->GetEndPoint();

	FVector Center = TargetData->GetOrigin().GetTranslation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	if (bIgnoreSelf) ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	
	TArray<FOverlapResult> OverlapResults;

	if (TraceObjType == EVA_Shape::None)
	{
		return {};
	}
	else if (TraceObjType == EVA_Shape::Sphere)
	{
		if (!bUseSector)
		{
			GetWorld()->OverlapMultiByChannel(
				OverlapResults, Center, FQuat::Identity, ECC_Pawn,
				FCollisionShape::MakeSphere(HalfSize.X));

			if (bDrawDebug)
				FDebugShapeHelper::DrawDebugSectorableCircle(GetWorld(), Center, HalfSize.X, 32,
					false, 0.f, GetAvatarActorFromActorInfo()->GetActorForwardVector(),
					FColor::White, 1.f);
		}
		else
		{
			GetWorld()->OverlapMultiByChannel(
				OverlapResults, Center, FQuat::Identity, ECC_Pawn,
				FCollisionShape::MakeSphere(HalfSize.X));

			if (bDrawDebug)
				FDebugShapeHelper::DrawDebugSectorableCircle(GetWorld(), Center, HalfSize.X, 360,
					true, SectorAngle, GetAvatarActorFromActorInfo()->GetActorForwardVector(),
					FColor::White, 1.f);
		}
	}
	else if (TraceObjType == EVA_Shape::Box)
	{
		FRotator BoxWorldRot = GetAvatarActorFromActorInfo()->GetActorRotation() + BoxRot;
		
		GetWorld()->OverlapMultiByChannel(
			OverlapResults, Center, BoxWorldRot.Quaternion(), ECC_Pawn,
			FCollisionShape::MakeBox(HalfSize));

		if (bDrawDebug)
			FDebugShapeHelper::DrawDebugRect(GetWorld(), Center, HalfSize.X, HalfSize.Y,
				BoxWorldRot.Vector(), FColor::White, 1.f);
	}
	
	for (const FOverlapResult& Result : OverlapResults)
	{
		// 무시당해야하는 하찮은 액터들 거르기
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
	if (!VS || !Loc) return {};

	FGameplayAbilityTargetDataHandle LocHandle;
	{
		auto* Copy = new FGameplayAbilityTargetData_LocationInfo(*Loc);
		LocHandle.Data.Add(TSharedPtr<FGameplayAbilityTargetData>(Copy));
	}
	
	// 2) 기존 범용 함수로 위임
	if (VS->Shape == EVA_Shape::Sphere)
	{
		return GetHitResultFromSweepLocationTargetData(
			LocHandle, FVector(VS->SphereRadius,0,0),
			VS->LocalRotation, VS->bUseSector, VS->SectorAngle,
			TargetTeam, EVA_Shape::Sphere, bDrawDebug, bIgnoreSelf);
	}
	else // Box
	{
		return GetHitResultFromSweepLocationTargetData(
			LocHandle, VS->BoxHalfSize,
			VS->LocalRotation, VS->bUseSector, VS->SectorAngle,
			TargetTeam, EVA_Shape::Box, bDrawDebug, bIgnoreSelf);
	}
}