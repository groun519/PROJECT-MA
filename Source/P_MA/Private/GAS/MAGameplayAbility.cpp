// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

#include "DebugShapeHelper.h"
#include "Engine/OverlapResult.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "P_MA/P_MA.h"
#include "VirtualSocketTargetData.h"


UMAGameplayAbility::UMAGameplayAbility()
{
	ActivationBlockedTags.AddTag(UMAAbilitySystemStatics::GetAbilityBlockTag());
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

TArray<FHitResult> UMAGameplayAbility::GetHitResultFromSweepLocationTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FVector HalfSize, FRotator BoxRot,
	bool bUseSector, float SectorAngle,
	int32 TargetRelationMask,
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
	const bool bDoSectorFilter = bUseSector && SectorAngle > 0.f;
	FVector SectorForward = FVector::ForwardVector;
	float SectorCosThreshold = -1.f;
	if (bDoSectorFilter)
	{
		if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
		{
			SectorForward = AvatarActor->GetActorForwardVector();
		}
		SectorForward.Z = 0.f;
		if (!SectorForward.Normalize())
		{
			SectorForward = FVector::ForwardVector;
		}
		SectorCosThreshold = FMath::Cos(FMath::DegreesToRadians(SectorAngle * 0.5f));
	}


	TArray<FOverlapResult> OverlapResults;
	if (TraceObjType == EVA_Shape::None)
	{
		return {};
	}
	else if (TraceObjType == EVA_Shape::Circle)
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
	else if (TraceObjType == EVA_Shape::Rect) // 사각형
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
		AActor* HitActor = Result.GetActor();
		if (!HitActor) continue;

		if (HitActors.Contains(HitActor)) continue;

		if (bDoSectorFilter)
		{
			FVector ToTarget = HitActor->GetActorLocation() - Center;
			ToTarget.Z = 0.f;
			if (!ToTarget.Normalize()) continue;

			const float Dot = FVector::DotProduct(SectorForward, ToTarget);
			if (Dot < SectorCosThreshold) continue;
		}

		// 중복 피격 방지
		if (HitActors.Contains(Result.GetActor())) continue;

		if (OwnerTeamInterface)
		{
			const ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitActor);
			if (!MATargetRelation::MatchesMask(TargetRelationMask, OtherActorTeamAttitude)) continue;
		}

		HitActors.Add(HitActor);
		OutResults.Add(Result);
	}
	TArray<FHitResult> OutHits;
	FDebugShapeHelper::ConvertOverlapsToHitResults(OutResults, OutHits);
	
	return OutHits;
}

TArray<FHitResult> UMAGameplayAbility::GetHitResultFromVirtualSocketTargetData(
	const FGameplayAbilityTargetDataHandle& Handle)
{
	return GetHitResultFromVirtualSocketTargetData(Handle, MATargetRelation::GetDefaultMask());
}

TArray<FHitResult> UMAGameplayAbility::GetHitResultFromVirtualSocketTargetData(
	const FGameplayAbilityTargetDataHandle& Handle,
	int32 OverrideTargetRelationMask)
{
	// 1) Virtual socket target data and location data extraction
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
	
	// 2) Reuse the shared sweep helper with an overridden target team
	TArray<FHitResult> OutHits;
	if (VS->Shape == EVA_Shape::Circle)
	{
		OutHits = GetHitResultFromSweepLocationTargetData(
			LocHandle, FVector(VS->SphereRadius,0,0),
			VS->LocalRotation, VS->bUseSector, VS->SectorAngle,
			OverrideTargetRelationMask, EVA_Shape::Circle, VS->bDrawDebug, VS->bIgnoreOwner);
	}
	else // Box
	{
		OutHits = GetHitResultFromSweepLocationTargetData(
			LocHandle, VS->BoxHalfSize,
			VS->LocalRotation, false, 0,
			OverrideTargetRelationMask, EVA_Shape::Rect, VS->bDrawDebug, VS->bIgnoreOwner);
	}

	// 3) Execute gameplay cues for each resolved hit actor
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
                                                             TSubclassOf<UGameplayEffect> GameplayEffect, int Level,
                                                             const FMADamageExecutionConfig* DamageConfig)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeDamageEffectSpec(GameplayEffect, Level, DamageConfig);
	if (!EffectSpecHandle.IsValid()) return;

	ApplyGameplayEffectSpecToHitResultActor(HitResult, EffectSpecHandle);
}

void UMAGameplayAbility::ApplyGameplayEffectSpecToHitResultActor(const FHitResult& HitResult,
                                                                 const FGameplayEffectSpecHandle& EffectSpecHandle)
{
	if (!EffectSpecHandle.IsValid()) return;

	FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	EffectContext.AddHitResult(HitResult);

	EffectSpecHandle.Data->SetContext(EffectContext);

	ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor()));
}

FGameplayEffectSpecHandle UMAGameplayAbility::MakeDamageEffectSpec(
	TSubclassOf<UGameplayEffect> GameplayEffect,
	int32 Level,
	const FMADamageExecutionConfig* DamageConfig)
{
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffect, Level);
	if (DamageConfig)
	{
		UMAAbilitySystemStatics::ApplyDamageExecutionConfig(EffectSpecHandle, *DamageConfig);
	}
	return EffectSpecHandle;
}
